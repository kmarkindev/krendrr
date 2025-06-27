#include "Render/Model.h"
#include <array>
#include <stdexcept>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace krendrr::Render
{
    void Model::Load(const std::string_view& ModelFileName, const LoadParams& Params)
    {
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
            throw std::runtime_error("Failed to load model. File is invalid.");

        std::string_view ModelDirectory {};
        {
            auto LastSlashPos = ModelFileName.find_last_of('/');
            if(LastSlashPos != std::string_view::npos)
            {
                ModelDirectory = ModelFileName.substr(0,  LastSlashPos);
            }
        }

        std::unordered_map<std::string, std::shared_ptr<Texture>> LoadedTextures {};

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
                aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[meshIndex]];

                struct Vertex
                {
                    glm::vec3 Position {};
                    glm::vec2 UV {};
                    glm::vec3 Normal {};
                    glm::vec3 Tangent {};
                };

                constexpr std::array MeshLayout = {
                    Mesh::VertexBufferLayout {
                        .Stride = 11 * sizeof(float),
                        .Offset = 0,
                        .Type = GL_FLOAT,
                        .Count = 3,
                    },
                    Mesh::VertexBufferLayout {
                        .Stride = 11 * sizeof(float),
                        .Offset = 3 * sizeof(float),
                        .Type = GL_FLOAT,
                        .Count = 2
                    },
                    Mesh::VertexBufferLayout {
                        .Stride = 11 * sizeof(float),
                        .Offset = 5 * sizeof(float),
                        .Type = GL_FLOAT,
                        .Count = 3
                    },
                    Mesh::VertexBufferLayout {
                        .Stride = 11 * sizeof(float),
                        .Offset = 8 * sizeof(float),
                        .Type = GL_FLOAT,
                        .Count = 3
                    },
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
                            Mesh->mNormals[vertIndex].z
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
                        Mesh->mTangents[vertIndex].z
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

                TexturedMesh& NewTexturedMesh = Meshes.emplace_back();
                NewTexturedMesh.Color = glm::vec3{
                    static_cast<double>(rand()) / (RAND_MAX + 1.0),
                    static_cast<double>(rand()) / (RAND_MAX + 1.0),
                    static_cast<double>(rand()) / (RAND_MAX + 1.0)
                };
                NewTexturedMesh.Mesh.LoadIndexed(MeshLayout, std::span<const Vertex> {Vertices}, Indices);

                aiMaterial* Material = Scene->mMaterials[Mesh->mMaterialIndex];

                auto LoadTexture = [&](aiTextureType Type, std::shared_ptr<Texture>& Texture, GLint Format)
                {
                    if(Material->GetTextureCount(Type) > 0)
                    {
                        aiString RelativeTexturePath {};
                        Material->GetTexture(Type, 0, &RelativeTexturePath);

                        std::string TexturePath { ModelDirectory.begin(), ModelDirectory.end() };
                        TexturePath.append("/");
                        TexturePath.append(RelativeTexturePath.C_Str());

                        auto LoadedTextureIter = LoadedTextures.find(TexturePath);
                        if(LoadedTextureIter != LoadedTextures.end())
                        {
                            Texture = LoadedTextureIter->second;
                        }
                        else
                        {
                            Texture = std::make_shared<Render::Texture>();
                            Texture->Load(TexturePath, {
                                .ApiFormat = Format,
                                .bFlipTexture = true,
                            });
                            LoadedTextures.insert({TexturePath, Texture});
                        }
                    }
                };

                LoadTexture(aiTextureType_DIFFUSE, NewTexturedMesh.BaseColorTexture, GL_SRGB8_ALPHA8);
                LoadTexture(aiTextureType_METALNESS, NewTexturedMesh.MetallicTexture, GL_R8);
                LoadTexture(aiTextureType_SHININESS, NewTexturedMesh.RoughnessTexture, GL_R8);
                LoadTexture(aiTextureType_NORMALS, NewTexturedMesh.NormalTexture, GL_RGB8);
            }
        }
    }

    std::span<const Model::TexturedMesh> Model::GetMeshes() const
    {
        return Meshes;
    }
}
