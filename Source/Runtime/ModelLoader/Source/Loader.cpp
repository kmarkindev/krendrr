#include "Runtime/ModelLoader/Loader.h"
#include <nvtx3/nvtx3.hpp>
#include <stdexcept>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include "Runtime/MipMapsGenerator/Generator.h"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include "Runtime/Renderer/Core/TexturedMesh/Mesh.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"
#include "Runtime/ThreadPool/ThreadPool.h"

namespace krendrr::Runtime::ModelLoader
{
    // a bunch of data shared between thread pool jobs
    struct LoadData
    {
        std::unordered_map<std::string, std::shared_ptr<Renderer::Core::Texture>> TextureCache {};
        std::vector<MipMapsGenerator::Generator::TextureToProcess> TexturesToGenerateMipMaps {};
        std::mutex TextureCacheMutex {};

        LoadResult Result {};
        std::mutex ResultMutex {};

        struct CommandListAndAllocator
        {
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};
        };
        std::vector<CommandListAndAllocator> CommandLists {};
        std::mutex CommandListsMutex {};

        std::mutex UploadBuffersMutex {};
        std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> UploadBuffers {};

    };

    void ProcessAiNodeJob(
        const RenderApi::Core::RenderApi& RenderApi,
        LoadData& LoadData,
        const aiNode* Node,
        const aiScene* Scene,
        std::string_view ModelDirectory,
        const LoadParams& Params,
        std::atomic_bool& bError
    )
    {
        if (bError.load(std::memory_order::relaxed))
            return;

        nvtx3::scoped_range ProcessAiNodeRange {"Load Model: Process Ai Node"};

        // Create local-to-world transformation matrix

        glm::mat4 GlobalTransform = glm::mat4(1.0f);
        {
            const aiNode* CurrentNode = Node;
            while(CurrentNode)
            {
                glm::mat4 CurrentTransform = glm::mat4(
                    CurrentNode->mTransformation.a1, CurrentNode->mTransformation.a2, CurrentNode->mTransformation.a3, CurrentNode->mTransformation.a4,
                    CurrentNode->mTransformation.b1, CurrentNode->mTransformation.b2, CurrentNode->mTransformation.b3, CurrentNode->mTransformation.b4,
                    CurrentNode->mTransformation.c1, CurrentNode->mTransformation.c2, CurrentNode->mTransformation.c3, CurrentNode->mTransformation.c4,
                    CurrentNode->mTransformation.d1, CurrentNode->mTransformation.d2, CurrentNode->mTransformation.d3, CurrentNode->mTransformation.d4
                );

                GlobalTransform = GlobalTransform * CurrentTransform;
                CurrentNode = CurrentNode->mParent;
            }
            GlobalTransform = glm::transpose(GlobalTransform);
        }

        for(unsigned meshIndex = 0; meshIndex < Node->mNumMeshes; meshIndex++)
        {
            if (bError.load(std::memory_order::relaxed))
                return;

            const aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[meshIndex]];

            struct Vertex
            {
                glm::vec3 Position {};
                glm::vec2 UV {};
                glm::vec3 Normal {};
                glm::vec3 Tangent {};
            };

            // Prepare mesh data before loading it into vertex and index buffers

            std::vector<Vertex> Vertices {};
            Vertices.reserve(500);

            std::vector<unsigned int> Indices {};
            Indices.reserve(500);

            for(unsigned vertIndex = 0; vertIndex < Mesh->mNumVertices; vertIndex++)
            {
                auto& [Position, UV, Normal, Tangent] = Vertices.emplace_back();

                Position = GlobalTransform * glm::vec4{
                    Mesh->mVertices[vertIndex].x,
                    Mesh->mVertices[vertIndex].y,
                    Mesh->mVertices[vertIndex].z,
                    1.0f
                };

                if(Mesh->HasNormals())
                {
                    Normal = {
                        Mesh->mNormals[vertIndex].x,
                        Mesh->mNormals[vertIndex].y,
                        Mesh->mNormals[vertIndex].z,
                    };

                    if (Params.bFlipNormals)
                        std::swap(Normal.y, Normal.z);
                }

                if(Mesh->HasTextureCoords(0))
                {
                    UV = {
                        Mesh->mTextureCoords[0][vertIndex].x,
                        Mesh->mTextureCoords[0][vertIndex].y,
                    };
                }

                Tangent = {
                    Mesh->mTangents[vertIndex].x,
                    Mesh->mTangents[vertIndex].y,
                    Mesh->mTangents[vertIndex].z,
                };

                if (Params.bFlipNormals)
                    std::swap(Tangent.y, Tangent.z);

                if (Params.bNegateNormalZ)
                {
                    Normal.z = -Normal.z;
                    Tangent.z = -Tangent.z;
                }
            }

            for(unsigned faceIndex = 0; faceIndex < Mesh->mNumFaces; faceIndex++)
            {
                const aiFace Face = Mesh->mFaces[faceIndex];

                for(unsigned faceIndexIndex = 0; faceIndexIndex < Face.mNumIndices; faceIndexIndex++)
                {
                    Indices.push_back(Face.mIndices[faceIndexIndex]);
                }
            }

            if (bError.load(std::memory_order::relaxed))
                return;

            // Create final object
            std::shared_ptr<Renderer::Core::TexturedMesh> NewTexturedMesh = std::make_shared<Renderer::Core::TexturedMesh>();

            NewTexturedMesh->SetMeshColor(glm::vec3{
                static_cast<double>(rand()) / (RAND_MAX + 1.0),
                static_cast<double>(rand()) / (RAND_MAX + 1.0),
                static_cast<double>(rand()) / (RAND_MAX + 1.0)
            });

            // Create command list for mesh and texture loading
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> NewCommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> NewCommandList {};

            if (FAILED(RenderApi.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&NewCommandAllocator))))
            {
                bError.store(true, std::memory_order::relaxed);
                // TODO: log error
                return;
            }

            if (FAILED(RenderApi.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY,
                NewCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&NewCommandList))))
            {
                bError.store(true, std::memory_order::relaxed);
                // TODO: log error
                return;
            }

            // Create Mesh
            {
                std::shared_ptr<Renderer::Core::Mesh> NewMesh = std::make_shared<Renderer::Core::Mesh>();
                NewTexturedMesh->AssignMesh(NewMesh);

                Renderer::Core::Mesh::MeshLoadOperation MeshLoadOperation = NewMesh->LoadIndexed(
                    RenderApi, *NewCommandList.Get(), RenderApi::Core::RenderApi::ContainerToBytes(Vertices), Indices);

                if (!MeshLoadOperation.WasSuccessful())
                {
                    // TODO: log error

                    bError.store(true, std::memory_order::relaxed);
                    return;
                }

                // Save mesh upload buffers
                {
                    nvtx3::mark("Save mesh upload buffers under mutex");
                    std::unique_lock Lock {LoadData.UploadBuffersMutex};

                    LoadData.UploadBuffers.push_back(MeshLoadOperation.VertexBufferUploadBuffer);
                    LoadData.UploadBuffers.push_back(MeshLoadOperation.IndexBufferUploadBuffer);
                }
            }

            // Import mesh textures, fill texture cache
            {
                if (bError.load(std::memory_order::relaxed))
                    return;

                const aiMaterial* Material = Scene->mMaterials[Mesh->mMaterialIndex];
                auto LoadTexture = [&](aiTextureType Type, Renderer::Core::TexturedMesh& TargetMesh, const std::string_view& TextureName) -> bool
                {
                    if(Material->GetTextureCount(Type) > 0)
                    {
                        aiString RelativeTexturePath {};
                        Material->GetTexture(Type, 0, &RelativeTexturePath);

                        std::string TexturePath {ModelDirectory};
                        TexturePath.append("/");
                        TexturePath.append(RelativeTexturePath.C_Str());

                        std::shared_ptr<Renderer::Core::Texture> Texture {};

                        std::unique_lock TextureCacheLock(LoadData.TextureCacheMutex);

                        if(auto LoadedTextureIter = LoadData.TextureCache.find(TexturePath); LoadedTextureIter != LoadData.TextureCache.end())
                        {
                            Texture = LoadedTextureIter->second;
                        }
                        else
                        {
                            // Since texture loading may take a while, do not keep the mutex ownership
                            TextureCacheLock.unlock();

                            Texture = std::make_shared<Renderer::Core::Texture>();

                            const Renderer::Core::Texture::TextureLoadOperation TextureLoadOperation = Texture->Load(RenderApi, *NewCommandList.Get(), TexturePath);

                            if (!TextureLoadOperation.WasSuccessful())
                            {
                                // TODO: log error
                                return false;
                            }

                            // Safe texture upload buffer
                            {
                                nvtx3::mark("Save texture upload buffer under mutex");
                                std::unique_lock UploadBuffersLock(LoadData.UploadBuffersMutex);

                                LoadData.UploadBuffers.push_back(TextureLoadOperation.TextureUploadBuffer);
                            }

                            TextureCacheLock.lock();

                            if (Texture->GetMipsCount() > 1)
                            {
                                LoadData.TexturesToGenerateMipMaps.push_back({
                                    .Resource = Texture->GetResource().Get(),
                                    .Format = Texture->GetFormat(),
                                    .MipZeroSize = Texture->GetSize().x,
                                    .MipMapCount = Texture->GetMipsCount(),
                                    .bShouldNormalize = Type == aiTextureType_NORMALS
                                });
                            }

                            LoadData.TextureCache.insert({TexturePath, Texture});
                        }

                        TargetMesh.AssignTexture(std::string{TextureName}, Texture);
                    }

                    // It is OK if there are no textures.
                    // But it is not if there are, but we can't load them.
                    return true;
                };

                if (!LoadTexture(aiTextureType_DIFFUSE, *NewTexturedMesh, Params.DiffuseTextureName))
                {
                    // TODO: log error
                    bError.store(true, std::memory_order::relaxed);
                    return;
                }

                if (!LoadTexture(aiTextureType_METALNESS, *NewTexturedMesh, Params.MetallicTextureName))
                {
                    // TODO: log error
                    bError.store(true, std::memory_order::relaxed);
                    return;
                }

                if (!LoadTexture(aiTextureType_SHININESS, *NewTexturedMesh, Params.RoughnessTextureName))
                {
                    // TODO: log error
                    bError.store(true, std::memory_order::relaxed);
                    return;
                }

                if (!LoadTexture(aiTextureType_NORMALS, *NewTexturedMesh, Params.NormalTextureName))
                {
                    // TODO: log error
                    bError.store(true, std::memory_order::relaxed);
                    return;
                }

                if (!LoadTexture(aiTextureType_EMISSIVE, *NewTexturedMesh, Params.EmissiveTextureName))
                {
                    // TODO: log error
                    bError.store(true, std::memory_order::relaxed);
                    return;
                }
            }

            // Push command list into Copy queue and save it
            {
                if (FAILED(NewCommandList->Close()))
                {
                    bError.store(true, std::memory_order::relaxed);
                    // TODO: log error
                    return;
                }

                ID3D12CommandList* CommandLists[] = {NewCommandList.Get()};
                RenderApi.GetCopyQueue()
                    ->ExecuteCommandLists(1, CommandLists);

                nvtx3::mark("Save command allocator and list under mutex");
                std::unique_lock Lock {LoadData.CommandListsMutex};
                LoadData.CommandLists.push_back({
                    .CommandAllocator = NewCommandAllocator,
                    .CommandList = NewCommandList,
                });
            }

            // Finally push new Textured Mesh into result
            {
                if (bError.load(std::memory_order::relaxed))
                    return;

                nvtx3::mark("Save final textured mesh object under mutex");
                std::unique_lock Lock{LoadData.ResultMutex};
                LoadData.Result.TexturedMeshes.push_back(NewTexturedMesh);
            }
        }
    }

    LoadResult LoadModel(const std::string_view& ModelFileName, const RenderApi::Core::RenderApi& RenderApi, const LoadParams& Params)
    {
        nvtx3::scoped_range LoadModelRange {"Load Model"};

        Microsoft::WRL::ComPtr<ID3D12Fence> Fence {};
        CHECKED(
            RenderApi.GetDevice()
                ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)),
            "Can't create fence"
        )

        // Create Assimp scene
        Assimp::Importer Importer {};
        const aiScene* Scene {};
        {
            nvtx3::scoped_range AssimpSceneLoadingRange {"Assimp scene loading"};

            unsigned Flags = aiProcess_Triangulate
                | aiProcess_GenNormals
                | aiProcess_FindInvalidData
                | aiProcess_CalcTangentSpace
                | aiProcess_JoinIdenticalVertices
                | aiProcess_SortByPType
                | aiProcess_RemoveComponent;

            if(Params.bFlipUVs)
                Flags |= aiProcess_FlipUVs;

            Scene = Importer.ReadFile(
                ModelFileName.data(),
                Flags
            );

            if(!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
            {
                // TODO: log error. throw std::runtime_error("Failed to load model. File is invalid.");
                return {};
            }
        }

        // Get model file directory to find relative texture files during texture loading
        std::string_view ModelDirectory {};
        {
            auto LastSlashPos = ModelFileName.find_last_of('/');
            if(LastSlashPos != std::string_view::npos)
            {
                ModelDirectory = ModelFileName.substr(0,  LastSlashPos);
            }
        }

        // Startup thread pool, collect import jobs into pool
        std::optional<ThreadPool::ThreadPool> LocalThreadPool {};
        ThreadPool::ThreadPool* ThreadPool {};

        if (Params.ThreadPool)
        {
            ThreadPool = Params.ThreadPool;
        }
        else
        {
            LocalThreadPool.emplace();
            LocalThreadPool->Initialize();

            ThreadPool = &LocalThreadPool.value();
        }

        LoadData LoadData {};
        std::atomic_bool bError {};

        {
            nvtx3::scoped_range PushJobsRange {"Pushing jobs"};

            std::queue<const aiNode*> ToProcess {};
            ToProcess.push(Scene->mRootNode);

            while(!ToProcess.empty())
            {
                const aiNode* Node = ToProcess.front();
                ToProcess.pop();

                ThreadPool->PushJob([&, Node]()
                {
                    ProcessAiNodeJob(
                        RenderApi,
                        LoadData,
                        Node,
                        Scene,
                        ModelDirectory,
                        Params,
                        bError
                    );
                });

                for(unsigned i = 0; i < Node->mNumChildren; i++)
                {
                    ToProcess.push(Node->mChildren[i]);
                }
            }
        }

        // First let thread pool finish all of it's jobs
        ThreadPool->WaitForAllJobs();

        {
            nvtx3::scoped_range WaitCopyQueueFenceRange {"Waiting d3d copy queue fence"};

            // Then wait for Copy Queue to finish all imports
            CHECKED(
                RenderApi.GetCopyQueue()
                    ->Signal(Fence.Get(), 1),
                "Can't signal fence"
            )

            CHECKED(
                Fence->SetEventOnCompletion(1, nullptr),
                "Can't wait on fence"
            )
        }

        // Let all CPU and GPU jobs to finish before error out,
        // we don't want to remove buffers while there are jobs that are using them
        if (bError)
        {
            // TODO: add error log
            return {};
        }

        // Generate Mip Maps
        if (!LoadData.TexturesToGenerateMipMaps.empty())
        {
            MipMapsGenerator::Generator Generator {};

            if (!Generator.GenerateMipMaps(RenderApi, LoadData.TexturesToGenerateMipMaps))
            {
                // TODO: add error log
                return {};
            }

            CHECKED(
                RenderApi.GetComputeQueue()
                    ->Signal(Fence.Get(), 2),
                "Can't signal fence"
            )

            CHECKED(
                Fence->SetEventOnCompletion(2, nullptr),
                "Can't wait on fence"
            )
        }

        // TODO: add success log

        LoadData.Result.bSuccess = true;
        return LoadData.Result;
    }
}
