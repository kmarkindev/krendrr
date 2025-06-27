#include "BlinnPhongApp.h"

void krendrr::BlinnPhong::BlinnPhongApp::BeforeMainLoop()
{
    AppBase::BeforeMainLoop();

    SDL_GL_SetSwapInterval(1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_FRAMEBUFFER_SRGB);

    SDL_SetWindowRelativeMouseMode(Window, true);

    Shader.Load("Content/blinnphong.vert", "Content/blinnphong.frag");
    Mp7Model.Load("Content/hk-mp7-a1/source/MP7_for_Sketchfab.fbx");
    Camera.SetPosition({-30, 30, -85});
}

void krendrr::BlinnPhong::BlinnPhongApp::AfterMainLoop()
{
    AppBase::AfterMainLoop();
}

void krendrr::BlinnPhong::BlinnPhongApp::OnKeyUp(const SDL_Event& Event)
{
    AppBase::OnKeyUp(Event);
    Camera.ReceiveKeyUpEvent(Event);
}

void krendrr::BlinnPhong::BlinnPhongApp::OnKeyDown(const SDL_Event& Event)
{
    AppBase::OnKeyDown(Event);
    Camera.ReceiveKeyDownEvent(Event);
}

void krendrr::BlinnPhong::BlinnPhongApp::OnMouseMove(const SDL_Event& Event)
{
    AppBase::OnMouseMove(Event);
    Camera.ReceiveMouseMoveEvent(Event);
}

void krendrr::BlinnPhong::BlinnPhongApp::Render(float DeltaTime)
{
    AppBase::Render(DeltaTime);

    int Width {};
    int Height{};
    SDL_GetWindowSize(Window, &Width, &Height);
    glViewport(0, 0, Width, Height);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 ModelMatrix = glm::mat4(1.0f);
    glm::mat4 ViewMatrix = Camera.GetViewMatrix();
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), static_cast<float>(Width) / static_cast<float>(Height), 0.1f, 1000.0f);
    glm::mat4 MVP = ProjMatrix * ViewMatrix * ModelMatrix;
    glm::mat3 NormalMatrix = glm::transpose(glm::inverse(ModelMatrix));

    Shader.Use();
    Shader.SetMatrix4("MVP", MVP);
    Shader.SetMatrix4("ModelMatrix", ModelMatrix);
    Shader.SetMatrix3("NormalMatrix", NormalMatrix);
    Shader.SetVec3("CameraPos", Camera.GetPosition());

    for (const Render::Model::TexturedMesh& TexturedMesh : Mp7Model.GetMeshes())
    {
        TexturedMesh.Mesh.BindVAO();

        Shader.SetVec3("Color", TexturedMesh.Color);

        Shader.SetInt("BaseColorTexture", 0);
        Shader.SetInt("MetallicTexture", 1);
        Shader.SetInt("RoughnessTexture", 2);
        Shader.SetInt("NormalsTexture", 3);

        if(TexturedMesh.BaseColorTexture)
            TexturedMesh.BaseColorTexture->ActivateTexture(0);
        if(TexturedMesh.MetallicTexture)
            TexturedMesh.MetallicTexture->ActivateTexture(1);
        if(TexturedMesh.RoughnessTexture)
            TexturedMesh.RoughnessTexture->ActivateTexture(2);
        if(TexturedMesh.NormalTexture)
            TexturedMesh.NormalTexture->ActivateTexture(3);

        glDrawElements(GL_TRIANGLES, TexturedMesh.Mesh.GetPrimitivesCount(), GL_UNSIGNED_INT, reinterpret_cast<void*>(TexturedMesh.Mesh.GetPrimitivesOffset()));

        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        glBindTextureUnit(2, 0);
        glBindTextureUnit(3, 0);
    }
}

void krendrr::BlinnPhong::BlinnPhongApp::Update(float DeltaTime)
{
    AppBase::Update(DeltaTime);
    Camera.Update(DeltaTime);
}
