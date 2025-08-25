#include "Runtime/Renderer/Deferred/DeferredRenderer.h"
#include <array>
#include "Runtime/ModelLoader/Loader.h"
#include "Runtime/Renderer/Core/Lights/PointLight.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::Initialize(std::shared_ptr<Core::Scene> NewScene)
{
    Scene = std::move(NewScene);

    std::array ShadersToLink = {
        Core::Shader::ShaderToLink{
            .Shader = GeometryPassShader,
            .VertexShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/GeometryPass/geometry_pass.vert",
            .FragmentShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/GeometryPass/geometry_pass.frag"
        },
        Core::Shader::ShaderToLink{
            .Shader = AmbientDirectionalLightPassShader,
            .VertexShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/LightPass/Quad/light_pass_quad.vert",
            .FragmentShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/LightPass/Quad/light_pass_ambient_directional.frag"
        },
        Core::Shader::ShaderToLink{
            .Shader = PointLightShadowShader,
            .VertexShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/ShadowPass/PointLight/point_light_shadow.vert",
            .FragmentShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/ShadowPass/PointLight/point_light_shadow.frag"
        },
        Core::Shader::ShaderToLink{
            .Shader = PointLightColorShader,
            .VertexShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/LightPass/Sphere/light_pass_point_light_sphere.vert",
            .FragmentShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/LightPass/Sphere/light_pass_sphere.frag"
        },
        Core::Shader::ShaderToLink{
            .Shader = PostProcessShader,
            .VertexShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/post_process.vert",
            .FragmentShader = "../Content/krendrr_runtime_renderer_deferred/Shaders/post_process.frag"
        }
    };
    if (!Core::Shader::AttachLinkAll(ShadersToLink))
    {
        // TODO: add error log
        return false;
    }

    if (!InitializeFullscreenQuadMesh())
    {
        // TODO: add error log
        return false;
    }

    if (!InitializePointLightUnitSphere())
    {
        // TODO: add error log
        return false;
    }

    // TODO: log success

    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::Render(const std::span<Core::SceneView>& SceneViews)
{
    for (const Core::SceneView& SceneView: SceneViews)
    {
        if (!UpdateGBufferForView(SceneView))
            return false;

        GeometryPass(SceneView);

        SetupGBufferForLightPass(SceneView);

        AmbientDirectionalLightPass(SceneView);

        PointLightVolumesPass(SceneView);

        PostProcessingPass(SceneView);
    }

    return true;
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::GeometryPass(const Core::SceneView& SceneView)
{
    glBindFramebuffer(GL_FRAMEBUFFER, GBufferFramebufferId);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const glm::mat4 ViewMatrix = SceneView.GetViewMatrix();
    const glm::mat4 ProjMatrix = SceneView.GetProjectionMatrix();

    for (auto Meshes = Scene->GetTexturedMeshes(); const auto& TexturedMesh : Meshes)
    {
        if(TexturedMesh->HasTexture(DIFFUSE_TEXTURE_NAME))
            TexturedMesh->GetTexture(DIFFUSE_TEXTURE_NAME)->ActivateTexture(0);
        if(TexturedMesh->HasTexture(METALLIC_TEXTURE_NAME))
            TexturedMesh->GetTexture(METALLIC_TEXTURE_NAME)->ActivateTexture(1);
        if(TexturedMesh->HasTexture(NORMAL_TEXTURE_NAME))
            TexturedMesh->GetTexture(NORMAL_TEXTURE_NAME)->ActivateTexture(2);
        if(TexturedMesh->HasTexture(ROUGHNESS_TEXTURE_NAME))
            TexturedMesh->GetTexture(ROUGHNESS_TEXTURE_NAME)->ActivateTexture(3);
        if (TexturedMesh->HasTexture(EMISSIVE_TEXTURE_NAME))
            TexturedMesh->GetTexture(EMISSIVE_TEXTURE_NAME)->ActivateTexture(4);

        GeometryPassShader.Use();
        GeometryPassShader.SetInt("BaseColorTexture", 0);
        GeometryPassShader.SetInt("MetallicTexture", 1);
        GeometryPassShader.SetInt("NormalTexture", 2);
        GeometryPassShader.SetInt("RoughnessTexture", 3);
        GeometryPassShader.SetInt("EmissiveTexture", 4);

        glm::mat4 ModelMatrix = TexturedMesh->GetModelMatrix();
        glm::mat4 MVPMatrix = ProjMatrix * ViewMatrix * ModelMatrix;
        glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));

        GeometryPassShader.Use();
        GeometryPassShader.SetMatrix4("MVPMatrix", MVPMatrix);
        GeometryPassShader.SetMatrix4("ModelMatrix", ModelMatrix);
        GeometryPassShader.SetMatrix3("NormalMatrix", NormalMatrix);

        TexturedMesh->GetMesh()->BindVAOAndDraw();

        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        glBindTextureUnit(2, 0);
        glBindTextureUnit(3, 0);
        glBindTextureUnit(4, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::SetupGBufferForLightPass(const Core::SceneView& SceneView)
{
    const glm::ivec2 SceneViewSize = SceneView.GetViewportSize();

    // Copy Depth from geometry pass
    glBlitNamedFramebuffer(
        GBufferFramebufferId,
        LightPassFramebufferId,
        0,
        0,
        SceneViewSize.x,
        SceneViewSize.y,
        0,
        0,
        SceneViewSize.x,
        SceneViewSize.y,
        GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );

    glBindFramebuffer(GL_FRAMEBUFFER, LightPassFramebufferId);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::AmbientDirectionalLightPass(const Core::SceneView& SceneView)
{
    const glm::ivec2 SceneViewSize = SceneView.GetViewportSize();

    glBindFramebuffer(GL_FRAMEBUFFER, LightPassFramebufferId);

    AmbientDirectionalLightPassShader.Use();
    BindGBufferTextures(AmbientDirectionalLightPassShader);
    AmbientDirectionalLightPassShader.SetVec2("ScreenSize", {SceneViewSize.x, SceneViewSize.y});
    AmbientDirectionalLightPassShader.SetVec3("CameraPos", SceneView.GetPosition());

    FullscreenQuadMesh.BindVAOAndDraw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::PointLightVolumesPass(const Core::SceneView& SceneView)
{
    const glm::mat4 SceneViewProjMatrix = SceneView.GetProjectionMatrix();
    const glm::mat4 SceneViewViewMatrix = SceneView.GetViewMatrix();
    const glm::ivec2 SceneViewSize = SceneView.GetViewportSize();

    for (const auto& PointLight : Scene->GetPointLights())
    {
        // TODO: we are not using layered rendering here. Need to benchmark it before implementing

        // TODO: Do not create shadow map textures and framebuffer every frame

        GLuint PointLightShadowCubeMap {};
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &PointLightShadowCubeMap);

        constexpr int SHADOW_MAP_SIZE = 2048;

        const float PointLightFarDistance = PointLight->GetDistance() + 1.f;

        glTextureStorage2D(PointLightShadowCubeMap, 1, GL_R16, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

        // Use linear to make shadows less pixelated
        glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        const std::array ShadowViewMatrices = {
            glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{1, 0, 0}, {0, -1, 0}),
            glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{-1, 0, 0}, {0, -1, 0}),
            glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, 1, 0}, {0, 0, 1}),
            glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, -1, 0}, {0, 0, -1}),
            glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, 0, 1}, {0, -1, 0}),
            glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, 0, -1}, {0, -1, 0}),
        };

        const glm::mat4 ShadowProjMatrix = glm::perspective(
            glm::radians(90.f),
            1.0f,
            0.01f,
            PointLightFarDistance
        );

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        GLuint ShadowMapFramebuffer {};
        glCreateFramebuffers(1, &ShadowMapFramebuffer);

        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glClearColor(1, 1, 1, 1);

        for (int i = 0; i < 6; ++i)
        {
            GLuint ShadowMapDepthBuffer {};
            glCreateRenderbuffers(1, &ShadowMapDepthBuffer);
            glNamedRenderbufferStorage(ShadowMapDepthBuffer, GL_DEPTH24_STENCIL8, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

            glNamedFramebufferRenderbuffer(ShadowMapFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ShadowMapDepthBuffer);
            glNamedFramebufferTextureLayer(ShadowMapFramebuffer, GL_COLOR_ATTACHMENT0, PointLightShadowCubeMap, 0, i);

            glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapFramebuffer);

            glm::mat4 ShadowViewProjMatrix = ShadowProjMatrix * ShadowViewMatrices[i];

            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            for (auto Meshes = Scene->GetTexturedMeshes(); const auto& TexturedMesh : Meshes)
            {
                if (!TexturedMesh->CanCastShadow())
                    continue;

                glm::mat4 ModelMatrix = TexturedMesh->GetModelMatrix();
                glm::mat4 MVPMatrix = ShadowViewProjMatrix * ModelMatrix;

                PointLightShadowShader.Use();

                PointLightShadowShader.SetMatrix4("ModelMatrix", ModelMatrix);
                PointLightShadowShader.SetMatrix4("MVPMatrix", MVPMatrix);
                PointLightShadowShader.SetVec3("LightPosition", PointLight->GetPosition());
                PointLightShadowShader.SetFloat("FarDistance", PointLightFarDistance);

                TexturedMesh->GetMesh()->BindVAOAndDraw();
            }

            glDeleteRenderbuffers(1, &ShadowMapDepthBuffer);
        }

        glClearColor(0.f, 0.f, 0.f, 0.f);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0, 0, SceneViewSize.x, SceneViewSize.y);

        glDeleteFramebuffers(1, &ShadowMapFramebuffer);

        // First setup sphere position and shader

        glBindFramebuffer(GL_FRAMEBUFFER, LightPassFramebufferId);

        PointLightColorShader.Use();

        // No need for distance calculations since we have it as a configuration value in point light
        const float AffectedDistance = PointLight->GetDistance();

        glm::mat4 ModelMatrix = glm::mat4(1.f);
        ModelMatrix = glm::translate(ModelMatrix, PointLight->GetPosition());
        ModelMatrix = glm::scale(ModelMatrix, {AffectedDistance, AffectedDistance, AffectedDistance});

        glm::mat4 MVPMatrix = SceneViewProjMatrix * SceneViewViewMatrix * ModelMatrix;

        int LastTextureUnit = BindGBufferTextures(PointLightColorShader);
        PointLightColorShader.SetMatrix4("MVPMatrix", MVPMatrix);
        PointLightColorShader.SetInt("PointLightShadowCubeMap", LastTextureUnit += 1);
        glBindTextureUnit(LastTextureUnit, PointLightShadowCubeMap);
        PointLightColorShader.SetVec2("ScreenSize", {SceneViewSize.x, SceneViewSize.y});
        PointLightColorShader.SetVec3("CameraPos", SceneView.GetPosition());
        PointLightColorShader.SetFloat("PointLightFarPlane", PointLightFarDistance);
        PointLightColorShader.SetVec3("PointLightPosition", PointLight->GetPosition());
        PointLightColorShader.SetVec3("PointLightDiffuseColor", PointLight->GetColor());
        PointLightColorShader.SetVec3("PointLightSpecularColor", PointLight->GetColor());
        PointLightColorShader.SetFloat("PointLightAttenuationLinear", PointLight->GetAttenuationLinear());
        PointLightColorShader.SetFloat("PointLightAttenuationQuad", PointLight->GetAttenuationQuad());
        PointLightColorShader.SetFloat("PointLightAttenuationConstant", PointLight->GetAttenuationConstant());

        // Now render ONLY to stencil

        glEnable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);

        glClear(GL_STENCIL_BUFFER_BIT);

        const auto& SphereMesh = PointLightUnitSphere->GetMesh();
        SphereMesh->BindVAOAndDraw();

        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // And finally render point light with stencil masking

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX); // resulting alpha is max(one, one) = one.

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glStencilMask(0x00);
        glStencilFunc(GL_EQUAL, 0x01, 0xFF);

        SphereMesh->BindVAOAndDraw();

        int LastUnbindId = UnbindGBufferTextures();
        glBindTextureUnit(LastUnbindId += 1, 0);
        glDeleteTextures(1, &PointLightShadowCubeMap);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glCullFace(GL_BACK);
        glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    }
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::PostProcessingPass(const Core::SceneView& SceneView)
{
    const glm::ivec2 ViewSize = SceneView.GetViewportSize();

    glBindFramebuffer(GL_FRAMEBUFFER, SceneView.GetFramebuffer());
    glClear(GL_COLOR_BUFFER_BIT);

    PostProcessShader.Use();
    int LastBindId = BindGBufferTextures(PostProcessShader);
    glBindTextureUnit(LastBindId += 1, LightPassColorTextureId);
    PostProcessShader.SetInt("FinalRenderTexture", LastBindId);
    PostProcessShader.SetVec2("ScreenSize", {ViewSize.x, ViewSize.y});

    FullscreenQuadMesh.BindVAOAndDraw();

    int LastUnbindId = UnbindGBufferTextures();
    PostProcessShader.SetInt("FinalRenderTexture", LastUnbindId += 1);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::Shutdown()
{
    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializePointLightUnitSphere()
{
    ModelLoader::LoadResult UnitSphereLoadResult = ModelLoader::LoadModel(
        "../Content/krendrr_runtime_renderer_deferred/UnitIcoSphere.obj"
    );

    if (!UnitSphereLoadResult.HasLoadedAtLeastOne())
    {
        // TODO: log error can't load unit sphere model
        return false;
    }

    PointLightUnitSphere = UnitSphereLoadResult.TexturedMeshes[0];

    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializeFullscreenQuadMesh()
{
    constexpr float QuadMesh[] = {
        -1.f, 1.f,
        -1.f, -1.f,
        1.f, -1.f,
        1.f, 1.f,
        -1.f, 1.f,
        1.f, -1.f
    };
    constexpr static std::array LayoutAttributes = {
        Core::Mesh::BufferLayoutAttribute {
            .Offset = 0,
            .Type = GL_FLOAT,
            .Count = 2
        }
    };
    constexpr static Core::Mesh::BufferLayout Layout = {
        .Stride = 2 * sizeof(float),
        .Attributes = LayoutAttributes
    };
    return FullscreenQuadMesh.Load(Layout, Core::Mesh::ContainerToBytes(QuadMesh));
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializeGBufferForView(const Core::SceneView& SceneView)
{
    // If we need to reinitialize GBuffer, first we should remove the old one
    if(GBufferFramebufferId > 0)
    {
        glDeleteTextures(1, &GBufferColorTextureId);
        glDeleteTextures(1, &GBufferPositionTextureId);
        glDeleteTextures(1, &GBufferNormalTextureId);
        glDeleteTextures(1, &GBufferMetallicTextureId);
        glDeleteTextures(1, &GBufferRoughnessTextureId);
        glDeleteTextures(1, &GBufferEmissiveTextureId);
        glDeleteRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
        glDeleteFramebuffers(1, &GBufferFramebufferId);
    }

    glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    glCreateFramebuffers(1, &GBufferFramebufferId);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferColorTextureId);
    glTextureStorage2D(GBufferColorTextureId, 1, GL_RGBA8, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferColorTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferColorTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferPositionTextureId);
    glTextureStorage2D(GBufferPositionTextureId, 1, GL_RGB32F, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferPositionTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferPositionTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferNormalTextureId);
    glTextureStorage2D(GBufferNormalTextureId, 1, GL_RGB32F, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferNormalTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferNormalTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferMetallicTextureId);
    glTextureStorage2D(GBufferMetallicTextureId, 1, GL_R8, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferMetallicTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferMetallicTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferRoughnessTextureId);
    glTextureStorage2D(GBufferRoughnessTextureId, 1, GL_R8, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferRoughnessTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferRoughnessTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferEmissiveTextureId);
    glTextureStorage2D(GBufferEmissiveTextureId, 1, GL_RGBA8, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferEmissiveTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferEmissiveTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
    glNamedRenderbufferStorage(GBufferDepthStencilRenderBufferId, GL_DEPTH24_STENCIL8, ViewportSize.x, ViewportSize.y);

    glNamedFramebufferRenderbuffer(GBufferFramebufferId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, GBufferDepthStencilRenderBufferId);
    glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT0, GBufferColorTextureId, 0);
    glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT1, GBufferPositionTextureId, 0);
    glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT2, GBufferNormalTextureId, 0);
    glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT3, GBufferMetallicTextureId, 0);
    glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT4, GBufferRoughnessTextureId, 0);
    glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT5, GBufferEmissiveTextureId, 0);

    if(glCheckNamedFramebufferStatus(GBufferFramebufferId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteTextures(1, &GBufferColorTextureId);
        glDeleteTextures(1, &GBufferPositionTextureId);
        glDeleteTextures(1, &GBufferNormalTextureId);
        glDeleteTextures(1, &GBufferMetallicTextureId);
        glDeleteTextures(1, &GBufferRoughnessTextureId);
        glDeleteTextures(1, &GBufferEmissiveTextureId);
        glDeleteRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
        glDeleteFramebuffers(1, &GBufferFramebufferId);

        // TODO: log error "GBuffer framebuffer is not complete"
        return false;
    }

    GLuint AttachmentsToEnable[6] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5
    };
    glNamedFramebufferDrawBuffers(GBufferFramebufferId, 6, AttachmentsToEnable);

    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializeLightPassBufferForView(const Core::SceneView& SceneView)
{
    // If we need to reinitialize GBuffer, first we should remove the old one
    if(LightPassFramebufferId > 0)
    {
        glDeleteTextures(1, &LightPassColorTextureId);
        glDeleteRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
        glDeleteFramebuffers(1, &LightPassFramebufferId);
    }

    glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    glCreateFramebuffers(1, &LightPassFramebufferId);

    glCreateRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
    glNamedRenderbufferStorage(LightPassDepthStencilRenderBufferId, GL_DEPTH24_STENCIL8, ViewportSize.x, ViewportSize.y);

    glCreateTextures(GL_TEXTURE_2D, 1, &LightPassColorTextureId);
    // Use GL_RGBA16F so we can use HDR colors for this buffer
    glTextureStorage2D(LightPassColorTextureId, 1, GL_RGBA16F, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(LightPassColorTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(LightPassColorTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glNamedFramebufferRenderbuffer(LightPassFramebufferId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, LightPassDepthStencilRenderBufferId);
    glNamedFramebufferTexture(LightPassFramebufferId, GL_COLOR_ATTACHMENT0, LightPassColorTextureId, 0);

    if(glCheckNamedFramebufferStatus(GBufferFramebufferId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteTextures(1, &LightPassColorTextureId);
        glDeleteRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
        glDeleteFramebuffers(1, &LightPassFramebufferId);

        // TODO: log error "Light pass framebuffer is not complete"
        return false;
    }

    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::UpdateGBufferForView(const Core::SceneView& SceneView)
{
    glm::ivec4 Viewport = SceneView.GetViewport();
    glViewport(Viewport.x, Viewport.y, Viewport.z, Viewport.w);

    if(!InitializeGBufferForView(SceneView))
    {
        // TODO: add error log
        return false;
    }

    if(!InitializeLightPassBufferForView(SceneView))
    {
        // TODO: add error log
        return false;
    }

    return true;
}

int krendrr::Runtime::Renderer::Deferred::DeferredRenderer::BindGBufferTextures(Core::Shader& ShaderToBind)
{
    glBindTextureUnit(0, GBufferColorTextureId);
    glBindTextureUnit(1, GBufferPositionTextureId);
    glBindTextureUnit(2, GBufferNormalTextureId);
    glBindTextureUnit(3, GBufferMetallicTextureId);
    glBindTextureUnit(4, GBufferRoughnessTextureId);
    glBindTextureUnit(5, GBufferEmissiveTextureId);

    ShaderToBind.SetInt("GBufferColorTexture", 0);
    ShaderToBind.SetInt("GBufferWorldPositionTexture", 1);
    ShaderToBind.SetInt("GBufferWorldNormalTexture", 2);
    ShaderToBind.SetInt("GBufferMetallicTexture", 3);
    ShaderToBind.SetInt("GBufferRoughnessTextureId", 4);
    ShaderToBind.SetInt("GBufferEmissiveTextureId", 5);

    return 5;
}

int krendrr::Runtime::Renderer::Deferred::DeferredRenderer::UnbindGBufferTextures()
{
    glBindTextureUnit(0, 0);
    glBindTextureUnit(1, 0);
    glBindTextureUnit(2, 0);
    glBindTextureUnit(3, 0);
    glBindTextureUnit(4, 0);
    glBindTextureUnit(5, 0);

    return 5;
}
