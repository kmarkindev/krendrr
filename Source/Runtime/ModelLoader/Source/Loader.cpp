#include "Runtime/ModelLoader/Loader.h"

#include <array>
#include <stdexcept>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include "Runtime/Renderer/Core/TexturedMesh/Mesh.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"

namespace krendrr::Runtime::ModelLoader
{
    LoadResult LoadModel(const std::string_view& ModelFileName, const LoadParams& Params)
    {
        LoadResult Result {};

        Assimp::Importer Importer {};

        unsigned Flags = aiProcess_Triangulate
            | aiProcess_CalcTangentSpace
            | aiProcess_JoinIdenticalVertices
            | aiProcess_SortByPType
            | aiProcess_RemoveComponent;

        if(Params.bFlipUVs)
            Flags |= aiProcess_FlipUVs;

        const aiScene* Scene = Importer.ReadFile(
            ModelFileName.data(),
            Flags
        );

        if(!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
        {
            // TODO: log error. throw std::runtime_error("Failed to load model. File is invalid.");
            return {};
        }

        std::string_view ModelDirectory {};
        {
            auto LastSlashPos = ModelFileName.find_last_of('/');
            if(LastSlashPos != std::string_view::npos)
            {
                ModelDirectory = ModelFileName.substr(0,  LastSlashPos);
            }
        }

        std::unordered_map<std::string, std::shared_ptr<Renderer::Core::Texture>> LoadedTextures {};

        std::vector<aiNode*> ToProcess {};
        ToProcess.reserve(10);
        ToProcess.push_back(Scene->mRootNode);

        while(!ToProcess.empty())
        {
            aiNode* Node = ToProcess.back();
            ToProcess.pop_back();

            for(unsigned i = 0; i < Node->mNumChildren; i++)
            {
                ToProcess.push_back(Node->mChildren[i]);
            }

            glm::mat4 GlobalTransform = glm::mat4(1.0f);
            aiNode* CurrentNode = Node;
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

            for(unsigned meshIndex = 0; meshIndex < Node->mNumMeshes; meshIndex++)
            {
                const aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[meshIndex]];

                struct Vertex
                {
                    glm::vec3 Position {};
                    glm::vec2 UV {};
                    glm::vec3 Normal {};
                    glm::vec3 Tangent {};
                };

                constexpr static std::array MeshLayoutAttributes {
                    Renderer::Core::Mesh::BufferLayoutAttribute {
                        .Offset = 0,
                        .Type = GL_FLOAT,
                        .Count = 3,
                    },
                    Renderer::Core::Mesh::BufferLayoutAttribute {
                        .Offset = 3 * sizeof(float),
                           .Type = GL_FLOAT,
                           .Count = 2
                    },
                    Renderer::Core::Mesh::BufferLayoutAttribute {
                        .Offset = 5 * sizeof(float),
                            .Type = GL_FLOAT,
                            .Count = 3
                    },
                    Renderer::Core::Mesh::BufferLayoutAttribute {
                        .Offset = 8 * sizeof(float),
                            .Type = GL_FLOAT,
                            .Count = 3
                    }
                };

                constexpr static Renderer::Core::Mesh::BufferLayout MeshLayout = {
                    .Stride = 11 * sizeof(float),
                    .Attributes = MeshLayoutAttributes
                };

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
                }

                for(unsigned faceIndex = 0; faceIndex < Mesh->mNumFaces; faceIndex++)
                {
                    aiFace Face = Mesh->mFaces[faceIndex];

                    for(unsigned faceIndexIndex = 0; faceIndexIndex < Face.mNumIndices; faceIndexIndex++)
                    {
                        Indices.push_back(Face.mIndices[faceIndexIndex]);
                    }
                }


                std::shared_ptr<Renderer::Core::TexturedMesh> NewTexturedMesh = std::make_shared<Renderer::Core::TexturedMesh>();
                NewTexturedMesh->SetMeshColor(glm::vec3{
                    static_cast<double>(rand()) / (RAND_MAX + 1.0),
                    static_cast<double>(rand()) / (RAND_MAX + 1.0),
                    static_cast<double>(rand()) / (RAND_MAX + 1.0)
                });

                std::shared_ptr<Renderer::Core::Mesh> NewMesh = std::make_shared<Renderer::Core::Mesh>();
                if (!NewMesh->LoadIndexed(MeshLayout, Renderer::Core::Mesh::ContainerToBytes(Vertices), Indices))
                {
                    // TODO: log error
                    return {};
                }

                NewTexturedMesh->AssignMesh(NewMesh);

                Result.TexturedMeshes.push_back(NewTexturedMesh);

                aiMaterial* Material = Scene->mMaterials[Mesh->mMaterialIndex];

                auto LoadTexture = [&](aiTextureType Type, Renderer::Core::TexturedMesh& TargetMesh, const std::string_view& TextureName, GLint Format) -> bool
                {
                    if(Material->GetTextureCount(Type) > 0)
                    {
                        aiString RelativeTexturePath {};
                        Material->GetTexture(Type, 0, &RelativeTexturePath);

                        std::string TexturePath { ModelDirectory.begin(), ModelDirectory.end() };
                        TexturePath.append("/");
                        TexturePath.append(RelativeTexturePath.C_Str());

                        std::shared_ptr<Renderer::Core::Texture> Texture {};

                        auto LoadedTextureIter = LoadedTextures.find(TexturePath);
                        if(LoadedTextureIter != LoadedTextures.end())
                        {
                            Texture = LoadedTextureIter->second;
                        }
                        else
                        {
                            Texture = std::make_shared<Renderer::Core::Texture>();

                            Renderer::Core::Texture::TextureLoadParams Params = {
                                .ApiFormat = Format,
                                .bFlipTexture = true,
                            };

                            if (Type == aiTextureType_EMISSIVE)
                            {
                                // dont use mipmaps for emissive to prevent rendering them in places they should no be
                                // may need to remove it, since it might have been bad low-res textures
                                Params.MipMapsCount = 1;
                                Params.TextureMinFilter = GL_LINEAR;
                            }

                            const bool bLoadResult = Texture->Load(TexturePath, Params);

                            if (!bLoadResult)
                            {
                                // TODO: log error
                                return false;
                            }

                            LoadedTextures.insert({TexturePath, Texture});
                        }

                        TargetMesh.AssignTexture(std::string{TextureName}, Texture);
                    }

                    // It is OK if there are no textures.
                    // But it is not if there are, but we can't load them.
                    return true;
                };

                if (!LoadTexture(aiTextureType_DIFFUSE, *NewTexturedMesh, Params.DiffuseTextureName, GL_SRGB8_ALPHA8))
                {
                    // TODO: log error
                    return {};
                }

                if (!LoadTexture(aiTextureType_METALNESS, *NewTexturedMesh, Params.MetallicTextureName, GL_R8))
                {
                    // TODO: log error
                    return {};
                }

                if (!LoadTexture(aiTextureType_SHININESS, *NewTexturedMesh, Params.RoughnessTextureName, GL_R8))
                {
                    // TODO: log error
                    return {};
                }

                if (!LoadTexture(aiTextureType_NORMALS, *NewTexturedMesh, Params.NormalTextureName, GL_RGB8))
                {
                    // TODO: log error
                    return {};
                }

                if (!LoadTexture(aiTextureType_EMISSIVE, *NewTexturedMesh, Params.EmissiveTextureName, GL_SRGB8_ALPHA8))
                {
                    // TODO: log error
                    return {};
                }
            }
        }

        Result.bSuccess = true;
        return Result;
    }
}
