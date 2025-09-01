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
        int FenceValue {};

        bool WaitDirectQueue();

    };
}

