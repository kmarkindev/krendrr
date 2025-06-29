#include "DeferredShadingApp.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

namespace krendrr::DeferredShading
{
    void DeferredShadingApp::BeforeMainLoop()
    {
        AppBase::BeforeMainLoop();

        SDL_HideCursor();
        SDL_SetWindowMouseGrab(Window, true);

        Camera.SetPosition({-234.753769, 132.926086, 199.352264});
        Camera.SetRotation({0.937867283, {-0.00230924808, -0.345554471, -0.0317467079}});

        GeometryPassShader.Load("Content/Shaders/GeometryPass/geometry_pass.vert", "Content/Shaders/GeometryPass/geometry_pass.frag");

        AmbientDirectionalLightPassShader.Load("Content/Shaders/LightPass/Quad/ambient_light_quad.vert",
            "Content/Shaders/LightPass/Quad/light_pass_ambient_directional.frag");
        PointLightPassShader.Load("Content/Shaders/LightPass/Sphere/ambient_light_sphere.vert", "Content/Shaders/LightPass/Sphere/light_pass_point.frag");

        PostProcessShader.Load("Content/Shaders/post_process.vert", "Content/Shaders/post_process.frag");

        EnvModel.Load("Content/FuturisticRoom/source/CyberPunkRoom.fbx");

        constexpr float QuadMesh[] = {
            -1, 1,
            -1, -1,
            1, -1,
            1, 1,
            -1, 1,
            1, -1
        };
        constexpr Render::Mesh::VertexBufferLayout Layout[] = {
            {
                .Stride = 2 * sizeof(float),
                .Offset = 0,
                .Type = GL_FLOAT,
                .Count = 2
            }
        };
        FullscreenQuadMesh.Load(Layout, Utils::BytesArray{QuadMesh});

        // Setup ImGui
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        ImGui_ImplSDL3_InitForOpenGL(Window, Context);
        ImGui_ImplOpenGL3_Init();
    }

    void DeferredShadingApp::AfterMainLoop()
    {
        AppBase::AfterMainLoop();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void DeferredShadingApp::OnEvent(const SDL_Event& Event)
    {
        AppBase::OnEvent(Event);
        ImGui_ImplSDL3_ProcessEvent(&Event);
    }

    void DeferredShadingApp::OnKeyUp(const SDL_Event& Event)
    {
        AppBase::OnKeyUp(Event);

        Camera.ReceiveKeyUpEvent(Event);
    }

    void DeferredShadingApp::OnKeyDown(const SDL_Event& Event)
    {
        AppBase::OnKeyDown(Event);

        Camera.ReceiveKeyDownEvent(Event);
    }

    void DeferredShadingApp::OnMouseMove(const SDL_Event& Event)
    {
        AppBase::OnMouseMove(Event);

        Camera.ReceiveMouseMoveEvent(Event);
    }

    void DeferredShadingApp::Render(float DeltaTime)
    {
        AppBase::Render(DeltaTime);

        static int PrevWidth {-1};
        static int PrevHeight {-1};
        int Width {};
        int Height {};
        SDL_GetWindowSize(Window, &Width, &Height);
        if(PrevHeight != Height || PrevWidth != Width)
        {
            glViewport(0, 0, Width, Height);
            InitializeGBuffer();
            InitializeLightPassBuffer();

            PrevHeight = Height;
            PrevWidth = Width;
        }

        // Geometry Pass

        glBindFramebuffer(GL_FRAMEBUFFER, GBufferFramebufferId);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto Meshes = EnvModel.GetMeshes(); const auto& TexturedMesh : Meshes)
        {
            if(TexturedMesh.BaseColorTexture)
                TexturedMesh.BaseColorTexture->ActivateTexture(0);
            if(TexturedMesh.MetallicTexture)
                TexturedMesh.MetallicTexture->ActivateTexture(1);
            if(TexturedMesh.NormalTexture)
                TexturedMesh.NormalTexture->ActivateTexture(2);
            if(TexturedMesh.RoughnessTexture)
                TexturedMesh.RoughnessTexture->ActivateTexture(3);

            GeometryPassShader.Use();
            GeometryPassShader.SetInt("BaseColorTexture", 0);
            GeometryPassShader.SetInt("MetallicTexture", 1);
            GeometryPassShader.SetInt("NormalTexture", 2);
            GeometryPassShader.SetInt("RoughnessTexture", 3);

            glm::mat4 ModelMatrix = glm::mat4(1.0f);
            glm::mat4 ViewMatrix = Camera.GetViewMatrix();
            glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), static_cast<float>(Width) / static_cast<float>(Height), 0.1f, 1000.0f);

            glm::mat4 MVPMatrix = ProjMatrix * ViewMatrix * ModelMatrix;
            glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));

            GeometryPassShader.Use();
            GeometryPassShader.SetMatrix4("MVPMatrix", MVPMatrix);
            GeometryPassShader.SetMatrix4("ModelMatrix", ModelMatrix);
            GeometryPassShader.SetMatrix3("NormalMatrix", NormalMatrix);

            TexturedMesh.Mesh.BindVAO();
            glDrawElements(GL_TRIANGLES, TexturedMesh.Mesh.GetPrimitivesCount(), GL_UNSIGNED_INT, reinterpret_cast<void*>(TexturedMesh.Mesh.GetPrimitivesOffset()));
            glBindVertexArray(0);

            glBindTextureUnit(0, 0);
            glBindTextureUnit(1, 0);
            glBindTextureUnit(2, 0);
            glBindTextureUnit(3, 0);
        }

        // Lighting Pass

        // Copy Depth from geometry pass
        glBlitNamedFramebuffer(GBufferFramebufferId, LightPassFramebufferId,
            0, 0, Width, Height, 0, 0, Width, Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, LightPassFramebufferId);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTextureUnit(0, GBufferColorTextureId);
        glBindTextureUnit(1, GBufferPositionTextureId);
        glBindTextureUnit(2, GBufferNormalTextureId);
        glBindTextureUnit(3, GBufferMetallicTextureId);

        // Render Ambient and Directional light

        AmbientDirectionalLightPassShader.Use();
        AmbientDirectionalLightPassShader.SetInt("GBufferColorTexture", 0);
        AmbientDirectionalLightPassShader.SetInt("GBufferWorldPositionTexture", 1);
        AmbientDirectionalLightPassShader.SetInt("GBufferWorldNormalTexture", 2);
        AmbientDirectionalLightPassShader.SetInt("GBufferMetallicTexture", 3);
        AmbientDirectionalLightPassShader.SetVec2("ScreenSize", {Width, Height});

        FullscreenQuadMesh.BindVAO();
        glDrawArrays(GL_TRIANGLES, 0, FullscreenQuadMesh.GetPrimitivesCount());

        // Render other lights using Light Volumes

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_COLOR, GL_DST_COLOR, GL_ONE, GL_ONE);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX); // resulting alpha is max(one, one) = one.

        // Render point lights
        // TODO:

        glDisable(GL_BLEND);

        // Post Processing Pass

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        PostProcessShader.Use();

        glBindTextureUnit(0, LightPassColorTextureId);
        PostProcessShader.SetInt("FinalRenderTexture", 0);
        PostProcessShader.SetVec2("ScreenSize", {Width, Height});

        FullscreenQuadMesh.BindVAO();
        glDrawArrays(GL_TRIANGLES, 0, FullscreenQuadMesh.GetPrimitivesCount());

        // ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void DeferredShadingApp::Update(float DeltaTime)
    {
        AppBase::Update(DeltaTime);

        // Position mouse at the center, so it never touches window edge and we always get mouse moves
        int Width {};
        int Height{};
        SDL_GetWindowSize(Window, &Width, &Height);
        SDL_WarpMouseInWindow(Window, Width / 2, Height / 2);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        Camera.Update(DeltaTime);

        if(GBufferFramebufferId > 0)
        {
            ImGui::Begin("GBuffer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowPos({ 0, 0 });

            if(ImGui::BeginTable("GBuffer Table", 2))
            {
                float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
                float ScaledWidth = 256 * AspectRatio;
                float ScaledHeight = 256;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Color");
                ImGui::Image(GBufferColorTextureId, {ScaledWidth, ScaledHeight}, {0, 1}, {1, 0});

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("World Position");
                ImGui::Image(GBufferPositionTextureId, {ScaledWidth, ScaledHeight}, {0, 1}, {1, 0});

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Normal");
                ImGui::Image(GBufferNormalTextureId, {ScaledWidth, ScaledHeight}, {0, 1}, {1, 0});

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Metallic");
                ImGui::Image(GBufferMetallicTextureId, {ScaledWidth, ScaledHeight}, {0, 1}, {1, 0});

                ImGui::EndTable();
            }

            ImGui::End();
        }

        ImGui::EndFrame();
    }

    void DeferredShadingApp::InitializeGBuffer()
    {
        // If we need to reinitialize GBuffer, first we should remove the old one
        if(GBufferFramebufferId > 0)
        {
            glDeleteTextures(1, &GBufferColorTextureId);
            glDeleteTextures(1, &GBufferPositionTextureId);
            glDeleteTextures(1, &GBufferNormalTextureId);
            glDeleteTextures(1, &GBufferMetallicTextureId);
            glDeleteRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
            glDeleteFramebuffers(1, &GBufferFramebufferId);
        }

        int Width {};
        int Height{};
        SDL_GetWindowSize(Window, &Width, &Height);

        glCreateFramebuffers(1, &GBufferFramebufferId);

        glCreateTextures(GL_TEXTURE_2D, 1, &GBufferColorTextureId);
        glTextureStorage2D(GBufferColorTextureId, 1, GL_RGBA8, Width, Height);
        glTextureParameteri(GBufferColorTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GBufferColorTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glCreateTextures(GL_TEXTURE_2D, 1, &GBufferPositionTextureId);
        glTextureStorage2D(GBufferPositionTextureId, 1, GL_RGBA32F, Width, Height);
        glTextureParameteri(GBufferPositionTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GBufferPositionTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glCreateTextures(GL_TEXTURE_2D, 1, &GBufferNormalTextureId);
        glTextureStorage2D(GBufferNormalTextureId, 1, GL_RGBA32F, Width, Height);
        glTextureParameteri(GBufferNormalTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GBufferNormalTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glCreateTextures(GL_TEXTURE_2D, 1, &GBufferMetallicTextureId);
        glTextureStorage2D(GBufferMetallicTextureId, 1, GL_RGBA8, Width, Height);
        glTextureParameteri(GBufferMetallicTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GBufferMetallicTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glCreateRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
        glNamedRenderbufferStorage(GBufferDepthStencilRenderBufferId, GL_DEPTH24_STENCIL8, Width, Height);

        glNamedFramebufferRenderbuffer(GBufferFramebufferId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, GBufferDepthStencilRenderBufferId);
        glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT0, GBufferColorTextureId, 0);
        glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT1, GBufferPositionTextureId, 0);
        glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT2, GBufferNormalTextureId, 0);
        glNamedFramebufferTexture(GBufferFramebufferId, GL_COLOR_ATTACHMENT3, GBufferMetallicTextureId, 0);

        if(glCheckNamedFramebufferStatus(GBufferFramebufferId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteTextures(1, &GBufferColorTextureId);
            glDeleteTextures(1, &GBufferPositionTextureId);
            glDeleteTextures(1, &GBufferNormalTextureId);
            glDeleteTextures(1, &GBufferMetallicTextureId);
            glDeleteRenderbuffers(1, &GBufferDepthStencilRenderBufferId);
            glDeleteFramebuffers(1, &GBufferFramebufferId);
            throw std::runtime_error("GBuffer framebuffer is not complete");
        }

        GLuint AttachmentsToEnable[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glNamedFramebufferDrawBuffers(GBufferFramebufferId, 4, AttachmentsToEnable);
    }

    void DeferredShadingApp::InitializeLightPassBuffer()
    {
        // If we need to reinitialize GBuffer, first we should remove the old one
        if(LightPassFramebufferId > 0)
        {
            glDeleteTextures(1, &LightPassColorTextureId);
            glDeleteRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
            glDeleteFramebuffers(1, &LightPassFramebufferId);
        }

        int Width {};
        int Height{};
        SDL_GetWindowSize(Window, &Width, &Height);

        glCreateFramebuffers(1, &LightPassFramebufferId);

        glCreateRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
        glNamedRenderbufferStorage(LightPassDepthStencilRenderBufferId, GL_DEPTH24_STENCIL8, Width, Height);

        glCreateTextures(GL_TEXTURE_2D, 1, &LightPassColorTextureId);
        // Use GL_RGBA16F so we can use HDR colors for this buffer
        glTextureStorage2D(LightPassColorTextureId, 1, GL_RGBA16F, Width, Height);
        glTextureParameteri(LightPassColorTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(LightPassColorTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glNamedFramebufferRenderbuffer(LightPassFramebufferId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, LightPassDepthStencilRenderBufferId);
        glNamedFramebufferTexture(LightPassFramebufferId, GL_COLOR_ATTACHMENT0, LightPassColorTextureId, 0);

        if(glCheckNamedFramebufferStatus(GBufferFramebufferId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteTextures(1, &LightPassColorTextureId);
            glDeleteRenderbuffers(1, &LightPassDepthStencilRenderBufferId);
            glDeleteFramebuffers(1, &LightPassFramebufferId);
            throw std::runtime_error("Light pass framebuffer is not complete");
        }
    }
}
