#include <array>
#include <chrono>
#include <iostream>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <windows.h>
#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"

void callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    auto source_str = [source]() -> std::string {
        switch (source)
        {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY:  return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER: return "OTHER";
            default: return "UNKNOWN";
        }
    }();

    auto type_str = [type]() {
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR: return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER:  return "MARKER";
            case GL_DEBUG_TYPE_OTHER: return "OTHER";
            default: return "UNKNOWN";
        }
    }();

    auto severity_str = [severity]() {
        switch (severity) {
            case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
            case GL_DEBUG_SEVERITY_LOW: return "LOW";
            case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
            case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
            default: return "UNKNOWN";
        }
    }();

    if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    std::cout << source_str       << ", "
                  << type_str     << ", "
                  << severity_str << ", "
                  << id           << ": "
                  << message      << std::endl;
}

void SetupOpenGlDebugPrints()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(callback, nullptr);
}

SDL_Window *Window {};
bool bShouldQuit {false};
float DeltaTime {};

bool bWasSpacePressed {};
bool bWasShiftPressed {};
bool bWasWPressed {};
bool bWasAPressed {};
bool bWasSPressed {};
bool bWasDPressed {};
bool bWasEPressed {};
bool bWasQPressed {};
glm::vec2 MouseMove {};

glm::vec3 CameraPosition {};
glm::quat CameraRotation {1, {}};

krendrr::render::Shader Shader {};
krendrr::render::Model Mp7Model {};

void LoadRenderer()
{
    Shader.Load("Content/blinnphong.vert", "Content/blinnphong.frag");
    Mp7Model.Load("Content/hk-mp7-a1/source/MP7_for_Sketchfab.fbx");
}

void Update(float DeltaTime)
{
    float CameraMoveSpeed = 50.f;
    float CameraRotationScale = 0.35f;
    float CameraRollSpeed = 45.f;

    glm::mat4 CameraRotMatrix = glm::mat4_cast(CameraRotation);
    glm::vec3 CameraUpVector = CameraRotMatrix * glm::vec4{0.f, 1.f, 0.f, 0.f};
    glm::vec3 CameraForwardVector = CameraRotMatrix * glm::vec4{0.f,0.f,1.f, 0.f};
    glm::vec3 CameraRightVector = CameraRotMatrix * glm::vec4{-1.f, 0.f, 0.f, 0.f};

    if(bWasSpacePressed)
        CameraPosition += CameraUpVector * CameraMoveSpeed * DeltaTime;

    if(bWasShiftPressed)
        CameraPosition += CameraUpVector * -CameraMoveSpeed * DeltaTime;

    if(bWasWPressed)
        CameraPosition += CameraForwardVector * CameraMoveSpeed * DeltaTime;
    if(bWasAPressed)
        CameraPosition += CameraRightVector * -CameraMoveSpeed * DeltaTime;
    if(bWasSPressed)
        CameraPosition += CameraForwardVector * -CameraMoveSpeed * DeltaTime;
    if(bWasDPressed)
        CameraPosition += CameraRightVector * CameraMoveSpeed * DeltaTime;

    glm::quat MouseXRotation = glm::angleAxis(-glm::radians(MouseMove.x * CameraRotationScale), CameraUpVector);
    glm::quat MouseYRotation = glm::angleAxis(-glm::radians(MouseMove.y * CameraRotationScale), CameraRightVector);
    CameraRotation = MouseYRotation * MouseXRotation * CameraRotation;

    if(bWasEPressed)
    {
        glm::quat CameraRollRight = glm::angleAxis(-glm::radians(CameraRollSpeed * DeltaTime), CameraForwardVector);
        CameraRotation = CameraRollRight * CameraRotation;
    }

    if(bWasQPressed)
    {
        glm::quat CameraRollLeft = glm::angleAxis(glm::radians(CameraRollSpeed * DeltaTime), CameraForwardVector);
        CameraRotation = CameraRollLeft * CameraRotation;
    }
}

void Render(float DeltaTime)
{
    int Width {};
    int Height{};
    SDL_GetWindowSize(Window, &Width, &Height);
    glViewport(0, 0, Width, Height);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 ModelMatrix = glm::mat4(1.0f);

    glm::vec3 CameraForwardVector = glm::mat4_cast(CameraRotation) * glm::vec4{0.f,0.f,1.f, 0.f};
    glm::vec3 CameraUpVector = glm::mat4_cast(CameraRotation) * glm::vec4{0.f, 1.f, 0.f, 0.f};
    glm::mat4 ViewMatrix = glm::lookAt(
        CameraPosition,
        CameraPosition + CameraForwardVector,
        CameraUpVector
    );

    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), static_cast<float>(Width) / static_cast<float>(Height), 0.1f, 1000.0f);
    glm::mat4 MVP = ProjMatrix * ViewMatrix * ModelMatrix;

    Shader.Use();
    Shader.SetMatrix4("MVP", MVP);

    for (const krendrr::render::Model::TexturedMesh& TexturedMesh : Mp7Model.GetMeshes())
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

void OnKeyDown(const SDL_Event& Event)
{
    switch (Event.key.key)
    {
        case SDLK_SPACE:
            bWasSpacePressed = true;
            break;
        case SDLK_LSHIFT:
            bWasShiftPressed = true;
            break;
        case SDLK_W:
            bWasWPressed = true;
            break;
        case SDLK_A:
            bWasAPressed = true;
            break;
        case SDLK_S:
            bWasSPressed = true;
            break;
        case SDLK_D:
            bWasDPressed = true;
            break;
        case SDLK_Q:
            bWasEPressed = true;
        break;
        case SDLK_E:
            bWasQPressed = true;
        break;
    }
}

void OnKeyUp(const SDL_Event& Event)
{
    switch (Event.key.key)
    {
        case SDLK_SPACE:
            bWasSpacePressed = false;
        break;
        case SDLK_LSHIFT:
            bWasShiftPressed = false;
        break;
        case SDLK_W:
            bWasWPressed = false;
        break;
        case SDLK_A:
            bWasAPressed = false;
        break;
        case SDLK_S:
            bWasSPressed = false;
        break;
        case SDLK_D:
            bWasDPressed = false;
        break;
        case SDLK_Q:
            bWasEPressed = false;
        break;
        case SDLK_E:
            bWasQPressed = false;
        break;
    }
}

void OnMouseMove(const SDL_Event& Event)
{
    MouseMove = {Event.motion.xrel, Event.motion.yrel};
}

int main()
{
    // Initialize

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    Window = SDL_CreateWindow(
        "Blinn-Phong Renderer using OpenGL and SDL",
        1200, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS
    );

    SDL_GLContext context = SDL_GL_CreateContext(Window);

    int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "GPU: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Driver: " << glGetString(GL_VERSION) << std::endl;

    SetupOpenGlDebugPrints();

    SDL_GL_SetSwapInterval(1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_FRAMEBUFFER_SRGB);

    //SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(Window, true);

    LoadRenderer();

    // Game Loop

    SDL_Event event;

    std::chrono::steady_clock::time_point LastRecordedTime = std::chrono::steady_clock::now();
    DeltaTime = 0.0;

    while(!bShouldQuit) {

        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    bShouldQuit = true;
                break;
                case SDL_EVENT_KEY_DOWN:
                    OnKeyDown(event);
                break;
                case SDL_EVENT_KEY_UP:
                    OnKeyUp(event);
                break;
                case SDL_EVENT_MOUSE_MOTION:
                    OnMouseMove(event);
                default:
                    break;
            }
        }

        Update(DeltaTime);

        Render(DeltaTime);

        SDL_GL_SwapWindow(Window);

        MouseMove = {};

        std::chrono::steady_clock::time_point NewRecordedTime = std::chrono::steady_clock::now();
        auto Difference = std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(NewRecordedTime - LastRecordedTime);
        DeltaTime = Difference.count();
        LastRecordedTime = NewRecordedTime;
    }

    // Deinitialize

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}