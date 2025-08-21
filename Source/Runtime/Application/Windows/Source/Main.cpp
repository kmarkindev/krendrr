#include <memory>
#include <string>

#include "Runtime/Application/Core/Application.h"
#include "Runtime/Application/Core/EntryPoint.h"
#include "Runtime/Application/Core/StartupArgs.h"
#include "Runtime/Application/Core/Events/MouseWheelEvent.h"
#include "Runtime/Application/Core/Events/MouseMoveEvent.h"
#include "Runtime/Application/Core/Events/KeyEvent.h"
#include "Runtime/Application/Core/Events/QuitEvent.h"
#include "Runtime/Application/Windows/WindowsWindow.h"
#include "SDL3/SDL_events.h"

krendrr::Runtime::Application::Core::Window* TryGetEventWindow(const SDL_Event& Event)
{
    SDL_Window* Window = SDL_GetWindowFromEvent(&Event);

    if (!Window)
        return nullptr;

    void* WindowObject = SDL_GetPointerProperty(
        SDL_GetWindowProperties(Window),
        krendrr::Runtime::Application::Windows::WindowsWindow::SLD_WINDOW_OBJECT_PROPERTY,
        nullptr
    );

    if (!WindowObject)
        return nullptr;

    return static_cast<krendrr::Runtime::Application::Core::Window*>(WindowObject);
}

int main(int Argc, char** Argv)
{
    krendrr::Runtime::Application::Core::StartupArgs StartupArgs {Argc, Argv};

    std::unique_ptr<krendrr::Runtime::Application::Core::Application> Application { krendrr::Runtime::Application::Core::ConstructApplicationInstance(StartupArgs) };

    if (!Application->Initialize())
        return -1;

    bool bHasTickFailed {};
    while (!bHasTickFailed && !Application->HasRequestedShutdown())
    {
        SDL_Event SdlEvent {};
        while (SDL_PollEvent(&SdlEvent)) {
            switch(SdlEvent.type) {
                case SDL_EVENT_QUIT:
                {
                    krendrr::Runtime::Application::Core::QuitEvent QuitEvent {};
                    QuitEvent.Window = TryGetEventWindow(SdlEvent);

                    Application->HandleQuitEvent(QuitEvent);

                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    static const std::string KeyNamePrefix = "Mouse Button ";
                    const std::string KeyName = KeyNamePrefix + std::to_string(SdlEvent.button.button);

                    krendrr::Runtime::Application::Core::KeyEvent KeyDownEvent {};
                    KeyDownEvent.Window = TryGetEventWindow(SdlEvent);
                    KeyDownEvent.State = SdlEvent.button.down
                        ? krendrr::Runtime::Application::Core::KeyEvent::KeyState::Pressed
                        : krendrr::Runtime::Application::Core::KeyEvent::KeyState::Released;
                    KeyDownEvent.Key = KeyName;

                    Application->HandleKeyEvent(KeyDownEvent);

                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                {
                    krendrr::Runtime::Application::Core::KeyEvent KeyDownEvent {};
                    KeyDownEvent.Window = TryGetEventWindow(SdlEvent);
                    KeyDownEvent.State = SdlEvent.key.down
                        ? krendrr::Runtime::Application::Core::KeyEvent::KeyState::Pressed
                        : krendrr::Runtime::Application::Core::KeyEvent::KeyState::Released;
                    KeyDownEvent.Key = SDL_GetKeyName(SdlEvent.key.key);;

                    Application->HandleKeyEvent(KeyDownEvent);

                    break;
                }
                case SDL_EVENT_MOUSE_MOTION:
                {
                    krendrr::Runtime::Application::Core::MouseMoveEvent MouseMoveEvent {};
                    MouseMoveEvent.Window = TryGetEventWindow(SdlEvent);
                    MouseMoveEvent.Position = {
                        SdlEvent.motion.xrel,
                        SdlEvent.motion.yrel
                    };
                    MouseMoveEvent.Delta = {
                        SdlEvent.wheel.x,
                        SdlEvent.wheel.y
                    };

                    Application->HandleMouseMoveEvent(MouseMoveEvent);

                    break;
                }
                case SDL_EVENT_MOUSE_WHEEL:
                {
                    krendrr::Runtime::Application::Core::MouseWheelEvent MouseWheelEvent {};
                    MouseWheelEvent.Window = TryGetEventWindow(SdlEvent);
                    MouseWheelEvent.Delta = {
                        SdlEvent.wheel.x,
                        SdlEvent.wheel.y
                    };

                    Application->HandleMouseWheelEvent(MouseWheelEvent);

                    break;

                }
                default:
                    break;
            }
        }

        bHasTickFailed = !Application->Tick();
    }

    if (!Application->Shutdown())
        return -1;

    return bHasTickFailed ? -1 : 0;
}
