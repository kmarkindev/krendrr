#pragma once

#include <memory>

#include "Runtime/Renderer/Core/Renderer.h"
#include "Runtime/Renderer/Core/Shader.h"
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

        bool Initialize(std::shared_ptr<Core::Scene> NewScene) override;

        bool Render(const std::span<Core::SceneView>& SceneViews) override;

        bool Shutdown() override;

    private:

        std::shared_ptr<Core::Scene> Scene {};

        Core::Shader GeometryPassShader {};
        Core::Shader AmbientDirectionalLightPassShader {};

        GLuint GBufferFramebufferId {};
        GLuint GBufferColorTextureId {};
        GLuint GBufferPositionTextureId {};
        GLuint GBufferNormalTextureId {};
        GLuint GBufferMetallicTextureId {};
        GLuint GBufferRoughnessTextureId {};
        GLuint GBufferEmissiveTextureId {};
        GLuint GBufferDepthStencilRenderBufferId {};

        std::shared_ptr<Core::TexturedMesh> PointLightUnitSphere {};
        Core::Shader PointLightShadowShader {};
        Core::Shader PointLightColorShader {};

        bool InitializePointLightUnitSphere();

        GLuint LightPassFramebufferId {};
        GLuint LightPassColorTextureId {};
        GLuint LightPassDepthStencilRenderBufferId {};

        Core::Mesh FullscreenQuadMesh {};
        bool InitializeFullscreenQuadMesh();

        Core::Shader PostProcessShader {};

        bool InitializeGBufferForView(const Core::SceneView& SceneView);
        bool InitializeLightPassBufferForView(const Core::SceneView& SceneView);
        bool UpdateGBufferForView(const Core::SceneView& SceneView);

        // Returns last used texture unit. +1 and start binding your textures if needed.
        int BindGBufferTextures(Core::Shader& ShaderToBind);
        // Returns last used texture unit. +1 and start unbinding your textures if needed.
        int UnbindGBufferTextures();

        void GeometryPass(const Core::SceneView& SceneView);

        void SetupGBufferForLightPass(const Core::SceneView& SceneView);

        void AmbientDirectionalLightPass(const Core::SceneView& SceneView);

        void PointLightVolumesPass(const Core::SceneView& SceneView);

        void PostProcessingPass(const Core::SceneView& SceneView);

    };
}

