#include "SDLAppBase/AppBase.h"
#include <glad/gl.h>
#include <iostream>
#include <glm/mat4x4.hpp>
#include <SDL3/SDL_init.h>

namespace krendrr::SDLAppBase
{

    static void callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
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

        std::cerr << source_str
            << ", "
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

    void AppBase::Initialize(const std::string_view& Title, const glm::ivec2& WindowSize)
    {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        Window = SDL_CreateWindow(
            Title.data(),
            WindowSize.x, WindowSize.y,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS
        );

        Context = SDL_GL_CreateContext(Window);
        if (gladLoadGL(SDL_GL_GetProcAddress) == 0) {
            throw std::runtime_error("Failed to initialize OpenGL context");
        }

        std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << "\n";
        std::cout << "GPU: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "OpenGL Driver: " << glGetString(GL_VERSION) << std::endl;

        SetupOpenGlDebugPrints();

        BeforeMainLoop();
    }

    void AppBase::Uninitialize()
    {
        AfterMainLoop();

        if(Context)
            SDL_GL_DestroyContext(Context);

        if(Window)
            SDL_DestroyWindow(Window);

        SDL_Quit();
    }

    void AppBase::BeforeMainLoop()
    {
    }

    void AppBase::AfterMainLoop()
    {
    }

    void AppBase::OnKeyUp(const SDL_Event& Event)
    {
    }

    void AppBase::OnKeyDown(const SDL_Event& Event)
    {
    }

    void AppBase::OnMouseMove(const SDL_Event& Event)
    {
    }

    void AppBase::Render(float DeltaTime)
    {
    }

    void AppBase::Update(float DeltaTime)
    {
    }

    void AppBase::ExecuteMainLoop()
    {
        SDL_Event event;

        std::chrono::steady_clock::time_point LastRecordedTime = std::chrono::steady_clock::now();
        float DeltaTime = 0.0;

        bool bShouldQuit { false };
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

            std::chrono::steady_clock::time_point NewRecordedTime = std::chrono::steady_clock::now();
            auto Difference = std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(NewRecordedTime - LastRecordedTime);
            DeltaTime = Difference.count();
            LastRecordedTime = NewRecordedTime;
        }
    }
}
