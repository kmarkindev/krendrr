#include <array>
#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <windows.h>
#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

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

    __debugbreak();
}

void SetupOpenGlDebugPrints()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(callback, nullptr);
}

bool bShouldQuit {false};

struct Object
{
    glm::vec3 Position {};
    glm::quat Rotation {};
    glm::vec3 Scale {};

    glm::vec3 Color {};

    krendrr::render::Mesh Mesh {};
};

std::vector<Object> Objects {};

krendrr::render::Shader Shader {};
krendrr::render::Texture Texture {};

void LoadRenderer()
{
    Shader.Load("Content/blinnphong.vert", "Content/blinnphong.frag");

    Objects.push_back({});
    Object& obj = Objects[0];

    obj.Color = glm::vec3(.5f, .3f, 1.0f);

    float vertices[] = {
        // positions          // colors           // texture coords
       //  0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
       //  0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
       // -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
       // -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left

        0.5f,  0.5f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,    // bottom right
       -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,   // bottom left
       -0.5f,  0.5f, 0.0f, 0.0f, 1.0f    // top left
   };
    std::uint32_t indices[] = {
        3, 1, 0, // first triangle
        3, 2, 1  // second triangle
    };

    obj.Mesh.LoadIndexed(
        std::array{
            krendrr::render::Mesh::VertexBufferLayout{
                .Stride = 5 * sizeof(float),
                .Offset = 0,
                .Type = GL_FLOAT,
                .Count = 3
            },
            krendrr::render::Mesh::VertexBufferLayout{
                .Stride = 5 * sizeof(float),
                .Offset = 3 * sizeof(float),
                .Type = GL_FLOAT,
                .Count = 2
            }
        },
        vertices,
        indices
    );

    Texture.Load("Content/Texture.png", {
        .TextureWrapS = GL_CLAMP_TO_EDGE,
        .TextureWrapT = GL_CLAMP_TO_EDGE,
        .bFlipTexture = true
    });
}

void Render()
{
    glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Objects[0].Mesh.BindVAO();

    Texture.ActivateTexture(0);

    Shader.Use();
    Shader.SetInt("texture1", 0);
    Shader.SetVec3("Color", Objects[0].Color);

    glDrawElements(GL_TRIANGLES, Objects[0].Mesh.GetPrimitivesCount(), GL_UNSIGNED_INT, reinterpret_cast<void*>(Objects[0].Mesh.GetPrimitivesOffset()));
}

void OnKeyDown(const SDL_Event& Event)
{

}

int main()
{
    // Initialize

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow(
        "Blinn-Phong Renderer using OpenGL and SDL",
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS
    );

    SDL_GLContext context = SDL_GL_CreateContext(window);

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

    LoadRenderer();

    // Game Loop

    SDL_Event event;

    while(!bShouldQuit) {

        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    bShouldQuit = true;
                break;
                case SDL_EVENT_KEY_DOWN:
                    OnKeyDown(event);
                break;
                default:
                    break;
            }
        }

        int width {};
        int height{};
        SDL_GetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);

        Render();

        SDL_GL_SwapWindow(window);
    }

    // Deinitialize

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}