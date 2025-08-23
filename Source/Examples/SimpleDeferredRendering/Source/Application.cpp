#include "Application.h"
#include <iostream>
#include <memory>
#include <glad/gl.h>
#include "Runtime/Application/Core/EntryPoint.h"
#include "Runtime/Application/Core/Window.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"

IMPLEMENT_ENTRY_POINT(krendrr::Examples::SimpleDeferredRendering::Application)

krendrr::Examples::SimpleDeferredRendering::Application::Application(const Runtime::Application::Core::StartupArgs& Args)
    : Runtime::Application::Core::Application(Args)
{

}

static void OpenGlDebugCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, GLchar const* Message, void const* UserParam)
    {
        auto SourceStr = [Source]()
        {
            switch (Source)
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

        auto TypeStr = [Type]()
        {
            switch (Type)
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

        auto SeverityStr = [Severity]()
        {
            switch (Severity) {
                case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
                case GL_DEBUG_SEVERITY_LOW: return "LOW";
                case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
                case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
                default: return "UNKNOWN";
            }
        }();

        if(Severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        // TODO: replace with spdlog
        std::cerr << SourceStr
            << ", "
            << TypeStr     << ", "
            << SeverityStr << ", "
            << Id           << ": "
            << Message      << std::endl;
    }


static void SetupOpenGlDebugPrints()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGlDebugCallback, nullptr);
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Initialize()
{
    Window = std::unique_ptr<Runtime::Application::Core::Window>{
        Runtime::Application::Core::Window::Create(this, {
            .Title = "Deferred Rendering Example",
            .Size = {1280, 720}
        })
    };

    if (!Window->CreateAndBindGlContext())
        return false;

    SetupOpenGlDebugPrints();

    // TODO: replace with spdlog
    std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "GPU: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Driver: " << glGetString(GL_VERSION) << std::endl;

    Scene = std::make_unique<Runtime::Renderer::Core::Scene>();

    Renderer = std::make_unique<Runtime::Renderer::Deferred::DeferredRenderer>();
    Renderer->Initialize(Scene.get());

    return true;
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Tick()
{
    return true;
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Shutdown()
{
    return true;
}
