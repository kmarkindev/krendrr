#include "Runtime/Renderer/Deferred/DeferredRenderer.h"
#include <array>
#include "Runtime/ModelLoader/Loader.h"
#include "Runtime/Renderer/Core/Lights/PointLight.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"
#include "nvtx3/nvtx3.hpp"

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::Initialize(std::shared_ptr<Core::Scene> NewScene)
{
    nvtx3::scoped_range InitRange {"Deferred Renderer: Initialize"};

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

    InitializePointLightShadowBuffers();

    // TODO: log success

    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::Render(const std::span<Core::SceneView>& SceneViews)
{
    nvtx3::scoped_range RenderRange {"Deferred Renderer: Render"};

    for (const Core::SceneView& SceneView: SceneViews)
    {
        nvtx3::scoped_range SceneViewRange {"SceneView Iteration"};

        if (!UpdateGBufferForView(SceneView))
            return false;

        GeometryPass(SceneView);

        SetupLightPassFromGBuffer(SceneView);

        AmbientDirectionalLightPass(SceneView);

        PointLightVolumesPass(SceneView);

        PostProcessingPass(SceneView);
    }

    return true;
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::GeometryPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range GeometryPassRange {"Geometry Pass"};

    glBindFramebuffer(GL_FRAMEBUFFER, GBufferFramebufferId);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const glm::mat4 ViewMatrix = SceneView.GetViewMatrix();
    const glm::mat4 ProjMatrix = SceneView.GetProjectionMatrix();

    for (auto Meshes = Scene->GetTexturedMeshes(); const auto& TexturedMesh : Meshes)
    {
        const bool bHasNormalMap = TexturedMesh->HasTexture(NORMAL_TEXTURE_NAME);

        /*
        if(TexturedMesh->HasTexture(DIFFUSE_TEXTURE_NAME))
            TexturedMesh->GetTexture(DIFFUSE_TEXTURE_NAME)->ActivateTexture(0);
        if(TexturedMesh->HasTexture(METALLIC_TEXTURE_NAME))
            TexturedMesh->GetTexture(METALLIC_TEXTURE_NAME)->ActivateTexture(1);
        if(bHasNormalMap)
            TexturedMesh->GetTexture(NORMAL_TEXTURE_NAME)->ActivateTexture(2);
        if(TexturedMesh->HasTexture(ROUGHNESS_TEXTURE_NAME))
            TexturedMesh->GetTexture(ROUGHNESS_TEXTURE_NAME)->ActivateTexture(3);
        if (TexturedMesh->HasTexture(EMISSIVE_TEXTURE_NAME))
            TexturedMesh->GetTexture(EMISSIVE_TEXTURE_NAME)->ActivateTexture(4);
        */

        GeometryPassShader.Use();
        GeometryPassShader.SetInt("BaseColorTexture", 0);
        GeometryPassShader.SetInt("MetallicTexture", 1);
        GeometryPassShader.SetInt("NormalTexture", 2);
        GeometryPassShader.SetInt("RoughnessTexture", 3);
        GeometryPassShader.SetInt("EmissiveTexture", 4);

        glm::mat4 ModelMatrix = TexturedMesh->GetModelMatrix();
        glm::mat4 MVPMatrix = ProjMatrix * ViewMatrix * ModelMatrix;
        glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));

        GeometryPassShader.SetBool("HasNormalMap", bHasNormalMap);
        GeometryPassShader.SetMatrix4("MVPMatrix", MVPMatrix);
        GeometryPassShader.SetMatrix4("ModelMatrix", ModelMatrix);
        GeometryPassShader.SetMatrix3("NormalMatrix", NormalMatrix);

        //TexturedMesh->GetMesh()->BindVAOAndDraw();

        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        glBindTextureUnit(2, 0);
        glBindTextureUnit(3, 0);
        glBindTextureUnit(4, 0);
    }

    nvtx3::mark("Reset OpenGL state");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::SetupLightPassFromGBuffer(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range LightPassSetupRange {"LightPass Setup"};

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
    nvtx3::scoped_range AmbientDirectionalPassRange {"Ambient Directional Light Pass"};

    const bool bHasAmbient = Scene->HasAmbientLight();
    const bool bHasDirectional = Scene->HasDirectionalLight();

    if (!bHasAmbient && !bHasDirectional)
        return;

    const auto& AmbientData = Scene->GetAmbientLightData();
    const auto& DirectionalData = Scene->GetDirectionalLightData();

    const glm::ivec2 SceneViewSize = SceneView.GetViewportSize();

    glBindFramebuffer(GL_FRAMEBUFFER, LightPassFramebufferId);

    AmbientDirectionalLightPassShader.Use();
    BindGBufferTextures(AmbientDirectionalLightPassShader);

    AmbientDirectionalLightPassShader.SetBool("HasAmbient", bHasAmbient);
    if (bHasAmbient)
    {
        AmbientDirectionalLightPassShader.SetFloat("AmbientIntensity", AmbientData.Intensity);
        AmbientDirectionalLightPassShader.SetVec3("AmbientColor", AmbientData.Color);
    }

    AmbientDirectionalLightPassShader.SetBool("HasDirectional", bHasDirectional);
    if (bHasDirectional)
    {
        AmbientDirectionalLightPassShader.SetFloat("DirectionalIntensity", DirectionalData.Intensity);
        AmbientDirectionalLightPassShader.SetVec3("DirectionalColor", DirectionalData.Color);
        AmbientDirectionalLightPassShader.SetVec3("DirectionalDir", DirectionalData.Direction);
    }

    AmbientDirectionalLightPassShader.SetVec2("ScreenSize", {SceneViewSize.x, SceneViewSize.y});
    AmbientDirectionalLightPassShader.SetVec3("CameraPos", SceneView.GetPosition());

    //FullscreenQuadMesh.BindVAOAndDraw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializePointLightShadowBuffers()
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &PointLightShadowCubeMap);

    glTextureStorage2D(PointLightShadowCubeMap, 1, GL_R16, POINT_LIGHT_SHADOW_MAP_SIZE, POINT_LIGHT_SHADOW_MAP_SIZE);

    // Use linear to make shadows less pixelated
    glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(PointLightShadowCubeMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; ++i)
    {
        GLuint& ShadowMapFramebuffer = ShadowMapFramebuffers[i];
        GLuint& ShadowMapDepthBuffer = ShadowMapDepthRenderBuffers[i];

        glCreateFramebuffers(1, &ShadowMapFramebuffer);

        glCreateRenderbuffers(1, &ShadowMapDepthBuffer);
        glNamedRenderbufferStorage(ShadowMapDepthBuffer, GL_DEPTH24_STENCIL8, POINT_LIGHT_SHADOW_MAP_SIZE, POINT_LIGHT_SHADOW_MAP_SIZE);

        glNamedFramebufferRenderbuffer(ShadowMapFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ShadowMapDepthBuffer);
        glNamedFramebufferTextureLayer(ShadowMapFramebuffer, GL_COLOR_ATTACHMENT0, PointLightShadowCubeMap, 0, i);
    }
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::DestroyPointLightShadowBuffers()
{
    glDeleteFramebuffers(6, ShadowMapFramebuffers.data());
    glDeleteRenderbuffers(6, ShadowMapDepthRenderBuffers.data());
    glDeleteTextures(1, &PointLightShadowCubeMap);
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::PointLightVolumesPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range PointLightPassRange {"Point Light Pass"};

    // TODO: we are not using layered rendering here. Need to benchmark it before implementing

    const glm::mat4 SceneViewProjMatrix = SceneView.GetProjectionMatrix();
    const glm::mat4 SceneViewViewMatrix = SceneView.GetViewMatrix();
    const glm::ivec2 SceneViewSize = SceneView.GetViewportSize();

    for (const auto& PointLight : Scene->GetPointLights())
    {
        nvtx3::scoped_range PointLightIterationRange {"Point Light Iteration"};

        // +1 to make sure it's not zero and >= than near plane
        const float PointLightFarDistance = PointLight->GetDistance() + 1.f;

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
            1.0f,
            PointLightFarDistance
        );

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glViewport(0, 0, POINT_LIGHT_SHADOW_MAP_SIZE, POINT_LIGHT_SHADOW_MAP_SIZE);
        glClearColor(1, 1, 1, 1);

        {
            nvtx3::scoped_range PointLightShadowMapRenderRange {"Shadow Cube Map Render"};

            for (int i = 0; i < 6; ++i)
            {
                nvtx3::scoped_range PointLightShadowMapFaceRenderRange {"Face Render"};

                GLuint& ShadowMapFramebuffer = ShadowMapFramebuffers[i];

                glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapFramebuffer);

                glm::mat4 ShadowViewProjMatrix = ShadowProjMatrix * ShadowViewMatrices[i];

                glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

                PointLightShadowShader.Use();
                PointLightShadowShader.SetVec3("LightPosition", PointLight->GetPosition());
                PointLightShadowShader.SetFloat("FarDistance", PointLightFarDistance);

                for (auto Meshes = Scene->GetTexturedMeshes(); const auto& TexturedMesh : Meshes)
                {
                    if (!TexturedMesh->CanCastShadow())
                        continue;

                    nvtx3::scoped_range PointLightShadowMapFaceRenderMeshDrawRange {"Mesh Draw"};

                    glm::mat4 ModelMatrix = TexturedMesh->GetModelMatrix();
                    glm::mat4 MVPMatrix = ShadowViewProjMatrix * ModelMatrix;

                    PointLightShadowShader.SetMatrix4("ModelMatrix", ModelMatrix);
                    PointLightShadowShader.SetMatrix4("MVPMatrix", MVPMatrix);

                    nvtx3::mark("Draw");
                    //TexturedMesh->GetMesh()->BindVAOAndDraw();
                }
            }
        }

        nvtx3::mark("Reset OpenGL state");
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glm::ivec4 SceneViewport = SceneView.GetViewport();
        glViewport(SceneViewport.x, SceneViewport.y, SceneViewport.z, SceneViewport.w);

        // First setup sphere position and shader

        glBindFramebuffer(GL_FRAMEBUFFER, LightPassFramebufferId);

        {
            nvtx3::scoped_range PointLightPassPrepareStencilColorRenderRange {"Prepare Stencil/Color Render"};

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
        }

        const auto& SphereMesh = PointLightUnitSphere->GetMesh();

        {
            nvtx3::scoped_range PointLightPassStencilRange {"Stencil Render"};

            // Now render ONLY to stencil

            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);

            glClear(GL_STENCIL_BUFFER_BIT);

            //SphereMesh->BindVAOAndDraw();

            nvtx3::mark("Reset OpenGL state");
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }

        {
            nvtx3::scoped_range PointLightPassStencilRange {"Light Render"};

            // And finally render point light with stencil masking

            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX); // resulting alpha is max(one, one) = one.

            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);

            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 0x01, 0xFF);

            //SphereMesh->BindVAOAndDraw();

            int LastUnbindId = UnbindGBufferTextures();
            glBindTextureUnit(LastUnbindId += 1, 0);

            nvtx3::mark("Reset OpenGL state");
            glDisable(GL_STENCIL_TEST);
            glStencilMask(0xFF);
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glCullFace(GL_BACK);
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
}

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::PostProcessingPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range PostProcessingPassRange {"PostProcessing Pass"};

    const glm::ivec2 ViewSize = SceneView.GetViewportSize();

    glBindFramebuffer(GL_FRAMEBUFFER, SceneView.GetFramebuffer());
    glClear(GL_COLOR_BUFFER_BIT);

    PostProcessShader.Use();
    int LastBindId = BindGBufferTextures(PostProcessShader);
    glBindTextureUnit(LastBindId += 1, LightPassColorTextureId);
    PostProcessShader.SetInt("FinalRenderTexture", LastBindId);
    PostProcessShader.SetVec2("ScreenSize", {ViewSize.x, ViewSize.y});

    //FullscreenQuadMesh.BindVAOAndDraw();

    nvtx3::mark("Reset OpenGL state");
    int LastUnbindId = UnbindGBufferTextures();
    glBindTextureUnit(LastUnbindId += 1, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::Shutdown()
{
    DestroyPointLightShadowBuffers();
    DestroyGBuffer();

    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializePointLightUnitSphere()
{
    /*
    ModelLoader::LoadResult UnitSphereLoadResult = ModelLoader::LoadModel(
        "../Content/krendrr_runtime_renderer_deferred/UnitIcoSphere.obj"
    );


    if (!UnitSphereLoadResult.HasLoadedAtLeastOne())
    {
        // TODO: log error can't load unit sphere model
        return false;
    }

    PointLightUnitSphere = UnitSphereLoadResult.TexturedMeshes[0];
    */
    return true;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializeFullscreenQuadMesh()
{
    /*
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

    */
    return false;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializeGBufferForView(const Core::SceneView& SceneView)
{
    glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    // No need to recreate GBuffer with same size
    if (GBufferSize == ViewportSize)
        return true;

    nvtx3::scoped_range GBufferInitRange {"GBuffer Init"};

    // If we need to reinitialize GBuffer, first we should remove the old one
    if(GBufferFramebufferId > 0)
    {
        DestroyGBuffer();
    }

    GBufferSize = ViewportSize;

    glCreateFramebuffers(1, &GBufferFramebufferId);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferColorTextureId);
    glTextureStorage2D(GBufferColorTextureId, 1, GL_RGBA16F, ViewportSize.x, ViewportSize.y);
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
    glTextureStorage2D(GBufferMetallicTextureId, 1, GL_R16F, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferMetallicTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferMetallicTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferRoughnessTextureId);
    glTextureStorage2D(GBufferRoughnessTextureId, 1, GL_R16F, ViewportSize.x, ViewportSize.y);
    glTextureParameteri(GBufferRoughnessTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GBufferRoughnessTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &GBufferEmissiveTextureId);
    glTextureStorage2D(GBufferEmissiveTextureId, 1, GL_RGBA16F, ViewportSize.x, ViewportSize.y);
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
        DestroyGBuffer();

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

void krendrr::Runtime::Renderer::Deferred::DeferredRenderer::DestroyGBuffer()
{
    glDeleteTextures(1, &GBufferColorTextureId);
    glDeleteTextures(1, &GBufferPositionTextureId);
    glDeleteTextures(1, &GBufferNormalTextureId);
    glDeleteTextures(1, &GBufferMetallicTextureId);
    glDeleteTextures(1, &GBufferRoughnessTextureId);
    glDeleteTextures(1, &GBufferEmissiveTextureId);
    glDeleteRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
    glDeleteFramebuffers(1, &GBufferFramebufferId);
    GBufferSize = {-1, -1};
    GBufferFramebufferId = 0;
}

bool krendrr::Runtime::Renderer::Deferred::DeferredRenderer::InitializeLightPassBufferForView(const Core::SceneView& SceneView)
{
    glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    if (ViewportSize == LightPassBufferSize)
        return true;

    nvtx3::scoped_range LightFramebufferRange {"LightPass Framebuffer Init"};

    // If we need to reinitialize GBuffer, first we should remove the old one
    if(LightPassFramebufferId > 0)
    {
        glDeleteTextures(1, &LightPassColorTextureId);
        glDeleteRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
        glDeleteFramebuffers(1, &LightPassFramebufferId);
        LightPassBufferSize = {-1, -1};
        LightPassFramebufferId = 0;
    }

    LightPassBufferSize = ViewportSize;

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
        LightPassBufferSize = {-1, -1};
        LightPassFramebufferId = 0;

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
