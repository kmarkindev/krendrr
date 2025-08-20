#pragma once

#include <chrono>
#include <glm/fwd.hpp>
#include <SDL3/SDL_events.h>

namespace krendrr::SDLAppBase
{

class AppBase 
{
public:

    virtual ~AppBase() = default;

    void Initialize(const std::string_view& Title, const glm::ivec2& WindowSize);

    void ExecuteMainLoop();
    void Uninitialize();

protected:

    SDL_Window *Window {};
    SDL_GLContext Context {};

    virtual void BeforeMainLoop();

    virtual void AfterMainLoop();

    virtual void OnEvent(const SDL_Event& Event);

    virtual void OnKeyUp(const SDL_Event& Event);

    virtual void OnKeyDown(const SDL_Event& Event);

    virtual void OnMouseMove(const SDL_Event& Event);

    virtual void OnMouseWheel(const SDL_Event& Event);

    virtual void Render(float DeltaTime);

    virtual void Update(float DeltaTime);

};

}
