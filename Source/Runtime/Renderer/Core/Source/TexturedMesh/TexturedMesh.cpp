#include "Runtime/Renderer/Core/TexturedMesh/TexturedMesh.h"
#include "glm/gtc/quaternion.hpp"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include "Runtime/RenderApi/Core/Builders/ConstBufferBuilder.h"

namespace krendrr::Runtime::Renderer::Core
{
    bool TexturedMesh::IsValid() const
    {
        return Mesh != nullptr;
    }

    void TexturedMesh::AssignMesh(std::shared_ptr<Core::Mesh> NewMesh)
    {
        Mesh = std::move(NewMesh);
    }

    void TexturedMesh::AssignTexture(std::string Name, std::shared_ptr<Texture> NewTexture)
    {
        Textures.insert_or_assign(std::move(Name), std::move(NewTexture));
    }

    std::shared_ptr<Texture> TexturedMesh::GetTexture(const std::string_view& Name) const
    {
        auto Iter = Textures.find(Name);

        if (Iter == Textures.end())
            return {};

        return Iter->second;
    }

    bool TexturedMesh::HasTexture(const std::string_view& Name) const
    {
        return Textures.contains(Name);
    }

    std::shared_ptr<Mesh> TexturedMesh::GetMesh() const
    {
        return Mesh;
    }

    const glm::vec3& TexturedMesh::GetMeshColor() const
    {
        return MeshColor;
    }

    void TexturedMesh::SetMeshColor(const glm::vec3& NewMeshColor)
    {
        MeshColor = NewMeshColor;
    }

    bool TexturedMesh::CanCastShadow() const
    {
        return bCanCastShadow;
    }

    void TexturedMesh::SetCanCastShadow(bool bNewCanCastShadow)
    {
        bCanCastShadow = bNewCanCastShadow;
    }

    const glm::vec3& TexturedMesh::GetPosition() const
    {
        return Position;
    }

    void TexturedMesh::SetPosition(const glm::vec3& NewPosition)
    {
        Position = NewPosition;
    }

    const glm::quat& TexturedMesh::GetRotation() const
    {
        return Rotation;
    }

    void TexturedMesh::SetRotation(const glm::quat& NewRotation)
    {
        Rotation = NewRotation;
    }

    const glm::vec3& TexturedMesh::GetScale() const
    {
        return Scale;
    }

    void TexturedMesh::SetScale(const glm::vec3& NewScale)
    {
        Scale = NewScale;
    }

    glm::mat4 TexturedMesh::GetModelMatrix() const
    {
        glm::mat4 ModelMatrix = glm::mat4(1.0f);

        // PS. Applied last -> first

        ModelMatrix = glm::translate(ModelMatrix, Position);
        ModelMatrix = glm::scale(ModelMatrix, Scale);
        ModelMatrix = glm::mat4_cast(Rotation) * ModelMatrix;

        return ModelMatrix;
    }

    glm::mat3 TexturedMesh::GetNormalMatrix() const
    {
        return glm::transpose(glm::inverse(glm::mat3(GetModelMatrix())));
    }

    bool TexturedMesh::UpdateConstantBuffer(const RenderApi::Core::RenderApi& RenderApi)
    {
        // Create buffer if not created
        if (ConstantBuffer == nullptr)
        {
            if (!BuildConstantBuffer<ConstBuff_TexturedMesh>(RenderApi, ConstantBuffer, CpuSrvHeap, L"Textured Mesh Constant Buffer"))
                return false;
        }

        // Update buffer

        ConstBuff_TexturedMesh* Buffer {};
        CHECKED_S(ConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&Buffer)))

        *Buffer = {
            .bHasNormalMap = HasTexture(NORMAL_TEXTURE_NAME),
            .ModelMatrix = GetModelMatrix(),
            .NormalMatrix = GetNormalMatrix(),
        };

        ConstantBuffer->Unmap(0, nullptr);

        return true;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE TexturedMesh::GetConstantBufferHandle() const
    {
        if (CpuSrvHeap == nullptr)
            return {};

        return CD3DX12_CPU_DESCRIPTOR_HANDLE {
            CpuSrvHeap->GetCPUDescriptorHandleForHeapStart()
        };
    }

    D3D12_GPU_VIRTUAL_ADDRESS TexturedMesh::GetConstantBufferGpuAddress() const
    {
        return ConstantBuffer->GetGPUVirtualAddress();
    }
}
