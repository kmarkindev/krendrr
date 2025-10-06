#pragma once

#include <map>
#include <memory>
#include <string>
#include <wrl/client.h>
#include <d3dx12/d3dx12.h>
#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"
#include "Runtime/RenderApi/Core/RenderApi.h"

namespace krendrr::Runtime::RenderApi::Core
{
    class StaticMesh;
    class Texture2D;
}

namespace krendrr::Runtime::Renderer::Core
{
    /**
     * Represents a textured mesh on scene
     *
     * Consists of a single mesh, set of textures and a bunch of parameters used by renderer
     */
    class TexturedStaticMesh
    {
    public:

        [[nodiscard]] bool IsValid() const;

        void AssignTexture(std::string Name, std::shared_ptr<RenderApi::Core::Texture2D> NewTexture);
        [[nodiscard]] std::shared_ptr<const RenderApi::Core::Texture2D> GetTexture(const std::string_view& Name) const;
        [[nodiscard]] bool HasTexture(const std::string_view& Name) const;

        void AssignMesh(std::shared_ptr<RenderApi::Core::StaticMesh> NewMesh);
        [[nodiscard]] std::shared_ptr<const RenderApi::Core::StaticMesh> GetMesh() const;
        [[nodiscard]] const glm::vec3& GetMeshColor() const;
        void SetMeshColor(const glm::vec3& NewMeshColor);

        [[nodiscard]] bool CanCastShadow() const;
        void SetCanCastShadow(bool bNewCanCastShadow);

        [[nodiscard]] const glm::vec3& GetPosition() const;
        void SetPosition(const glm::vec3& NewPosition);

        [[nodiscard]] const glm::quat& GetRotation() const;
        void SetRotation(const glm::quat& NewRotation);

        [[nodiscard]] const glm::vec3& GetScale() const;
        void SetScale(const glm::vec3& NewScale);

        [[nodiscard]] glm::mat4 GetModelMatrix() const;
        [[nodiscard]] glm::mat3 GetNormalMatrix() const;

        bool UpdateConstantBuffer(const RenderApi::Core::RenderApi& RenderApi);
        D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferHandle() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGpuAddress() const;

    private:

        glm::vec3 Position {};
        glm::quat Rotation {};
        glm::vec3 Scale {1.f, 1.f, 1.f};

        std::shared_ptr<const RenderApi::Core::StaticMesh> Mesh {};
        std::map<std::string, std::shared_ptr<const RenderApi::Core::Texture2D>, std::less<>> Textures {};

        glm::vec3 MeshColor {};
        bool bCanCastShadow {true};

        constexpr inline static const char* NORMAL_TEXTURE_NAME = "normal";

        struct alignas(256) ConstBuff_TexturedMesh
        {
            std::uint32_t bHasNormalMap {};
            std::byte Padding[12];

            glm::mat4 ModelMatrix {};

            glm::mat3x4 NormalMatrix {};
        };

        Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffer {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvHeap {};

    };
}
