#include "DeferredShadingApp.h"

namespace krendrr::DeferredShading
{
    void DeferredShadingApp::BeforeMainLoop()
    {
        AppBase::BeforeMainLoop();

        SDL_SetWindowRelativeMouseMode(Window, true);

        Camera.SetPosition({0, 35, 0});

        GeometryPassShader.Load("Content/Shaders/geometry_pass.vert", "Content/Shaders/geometry_pass.frag");
        AsianCityModel.Load("Content/AsianCity/AsianCity.fbx");
    }

    void DeferredShadingApp::AfterMainLoop()
    {
        AppBase::AfterMainLoop();
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

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        int Width {};
        int Height{};
        SDL_GetWindowSize(Window, &Width, &Height);
        glViewport(0, 0, Width, Height);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto Meshes = AsianCityModel.GetMeshes(); const auto& TexturedMesh : Meshes)
        {
            if(TexturedMesh.BaseColorTexture)
                TexturedMesh.BaseColorTexture->ActivateTexture(0);
            if(TexturedMesh.MetallicTexture)
                TexturedMesh.BaseColorTexture->ActivateTexture(1);
            if(TexturedMesh.NormalTexture)
                TexturedMesh.BaseColorTexture->ActivateTexture(2);
            if(TexturedMesh.RoughnessTexture)
                TexturedMesh.BaseColorTexture->ActivateTexture(3);

            GeometryPassShader.Use();
            GeometryPassShader.SetInt("BaseColorTexture", 0);
            GeometryPassShader.SetInt("MetallicTexture", 1);
            GeometryPassShader.SetInt("NormalTexture", 2);
            GeometryPassShader.SetInt("RoughnessTexture", 3);

            glm::mat4 ModelMatrix = glm::mat4(1.0f);
            glm::mat4 ViewMatrix = Camera.GetViewMatrix();
            glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), static_cast<float>(Width) / static_cast<float>(Height), 0.1f, 1000.0f);

            glm::mat4 MVPMatrix = ProjMatrix * ViewMatrix * ModelMatrix;
            glm::mat3 NormalMatrix = glm::transpose(glm::inverse(ModelMatrix));

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
    }

    void DeferredShadingApp::Update(float DeltaTime)
    {
        AppBase::Update(DeltaTime);
        Camera.Update(DeltaTime);
    }
}
