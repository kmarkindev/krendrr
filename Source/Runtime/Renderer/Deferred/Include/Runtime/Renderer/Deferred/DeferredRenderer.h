#pragma once

#include <array>
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
        glm::ivec2 GBufferSize {-1, -1};

        bool InitializeGBufferForView(const Core::SceneView& SceneView);
        bool UpdateGBufferForView(const Core::SceneView& SceneView);
        void DestroyGBuffer();

        std::shared_ptr<Core::TexturedMesh> PointLightUnitSphere {};
        Core::Shader PointLightShadowShader {};
        Core::Shader PointLightColorShader {};

        bool InitializePointLightUnitSphere();

        GLuint LightPassFramebufferId {};
        GLuint LightPassColorTextureId {};
        GLuint LightPassDepthStencilRenderBufferId {};
        glm::ivec2 LightPassBufferSize {-1, -1};

        bool InitializeLightPassBufferForView(const Core::SceneView& SceneView);

        Core::Mesh FullscreenQuadMesh {};
        bool InitializeFullscreenQuadMesh();

        Core::Shader PostProcessShader {};

        // Returns last used texture unit. +1 and start binding your textures if needed.
        int BindGBufferTextures(Core::Shader& ShaderToBind);
        // Returns last used texture unit. +1 and start unbinding your textures if needed.
        int UnbindGBufferTextures();

        void GeometryPass(const Core::SceneView& SceneView);

        void SetupLightPassFromGBuffer(const Core::SceneView& SceneView);

        void AmbientDirectionalLightPass(const Core::SceneView& SceneView);

        constexpr inline static int POINT_LIGHT_SHADOW_MAP_SIZE = 1024;
        GLuint PointLightShadowCubeMap {};
        std::array<GLuint, 6> ShadowMapFramebuffers {};
        std::array<GLuint, 6> ShadowMapDepthRenderBuffers {};

        void InitializePointLightShadowBuffers();
        void DestroyPointLightShadowBuffers();
        void PointLightVolumesPass(const Core::SceneView& SceneView);

        void PostProcessingPass(const Core::SceneView& SceneView);

    };
}

