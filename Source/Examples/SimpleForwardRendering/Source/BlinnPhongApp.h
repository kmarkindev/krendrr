#pragma once

#include <SDLAppBase/AppBase.h>
#include "Render/Model.h"
#include "Render/Shader.h"
#include "SDLAppBase/Camera.h"

namespace krendrr::BlinnPhong
{
    class BlinnPhongApp : public SDLAppBase::AppBase
    {
    protected:

        Render::Shader Shader {};
        Render::Model Mp7Model {};
        SDLAppBase::Camera Camera {};

        void BeforeMainLoop() override;

        void AfterMainLoop() override;

        void OnKeyUp(const SDL_Event& Event) override;

        void OnKeyDown(const SDL_Event& Event) override;

        void OnMouseMove(const SDL_Event& Event) override;

        void Render(float DeltaTime) override;

        void Update(float DeltaTime) override;
    };
};
