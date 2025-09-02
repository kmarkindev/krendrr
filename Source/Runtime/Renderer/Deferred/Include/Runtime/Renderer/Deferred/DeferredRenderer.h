#pragma once

#include <array>
#include <memory>
#include "Runtime/Renderer/Core/Renderer.h"
#include "Runtime/Renderer/Core/TexturedMesh/Mesh.h"
#include "Runtime/Renderer/Core/TexturedMesh/TexturedMesh.h"

namespace krendrr::Runtime::Renderer::Deferred
{
    class DeferredRenderer final : public Core::Renderer
    {
    public:

        constexpr inline static const char* DIFFUSE_TEXTURE_NAME = "diffuse";
        constexpr inline static const char* METALLIC_TEXTURE_NAME = "metallic";
        constexpr inline static const char* ROUGHNESS_TEXTURE_NAME = "roughness";
        constexpr inline static const char* NORMAL_TEXTURE_NAME = "normal";
        constexpr inline static const char* EMISSIVE_TEXTURE_NAME = "emissive";

        bool Initialize(std::shared_ptr<RenderApi::Core::RenderApi> NewRenderApi, std::shared_ptr<Core::Scene> NewScene) override;

        bool Render(const std::span<Core::SceneView>& SceneViews) override;

        bool Shutdown() override;

    private:

        std::shared_ptr<RenderApi::Core::RenderApi> RenderApi {};
        std::shared_ptr<Core::Scene> Scene {};

        Microsoft::WRL::ComPtr<ID3D12Fence> RenderFence {};
        int RenderFenceValue {};

        bool WaitDirectQueue();

        struct GBuffer
        {
            glm::ivec2 Size {-1, -1};

            Microsoft::WRL::ComPtr<ID3D12Resource> DiffuseTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> WorldPositionTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> WorldNormalTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> MetallicTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> RoughnessTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> EmissiveTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilTexture {};

            // Descriptors follow same order as textures are declared in this struct
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuRtvDescriptorHeap {};
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvDescriptorHeap {};
            inline constexpr static int TEXTURES_COUNT = 6;

            // Contains only one descriptor
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuDsvDescriptorHeap {};
        };

        GBuffer GBuffer {};

        struct GBufferInitResult
        {
            bool bSuccess {};
            bool bRecordedCommands {};
        };

        // Called every time we need to update GBuffer, so it has same size as scene view
        GBufferInitResult InitGBufferForView(const Core::SceneView& SceneView, ID3D12GraphicsCommandList* CommandList);

    };
}

