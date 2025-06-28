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

    GLuint GBufferFramebufferId {};
    GLuint GBufferColorTextureId {};
    GLuint GBufferPositionTextureId {};
    GLuint GBufferNormalTextureId {};
    GLuint GBufferMetallicTextureId {};
    GLuint GBufferDepthStencilTextureId {};

    void BeforeMainLoop() override;

    void AfterMainLoop() override;

    void OnEvent(const SDL_Event& Event) override;

    void OnKeyUp(const SDL_Event& Event) override;

    void OnKeyDown(const SDL_Event& Event) override;

    void OnMouseMove(const SDL_Event& Event) override;

    void Render(float DeltaTime) override;

    void Update(float DeltaTime) override;

private:

    void InitializeGBuffer();

};

}
