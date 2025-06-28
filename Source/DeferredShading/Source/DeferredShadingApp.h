#pragma once

#include <SDLAppBase/AppBase.h>
#include <SDLAppBase/Camera.h>
#include "Render/Model.h"
#include "Render/Shader.h"

namespace krendrr::DeferredShading
{

class DeferredShadingApp : public SDLAppBase::AppBase
{
protected:

    SDLAppBase::Camera Camera {};
    Render::Model AsianCityModel {};
    Render::Shader GeometryPassShader {};

    void BeforeMainLoop() override;

    void AfterMainLoop() override;

    void OnKeyUp(const SDL_Event& Event) override;

    void OnKeyDown(const SDL_Event& Event) override;

    void OnMouseMove(const SDL_Event& Event) override;

    void Render(float DeltaTime) override;

    void Update(float DeltaTime) override;
};

}
