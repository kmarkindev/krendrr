#pragma once

#include <SDLAppBase/AppBase.h>
#include <SDLAppBase/Camera.h>

#include "PointLight.h"
#include "Render/Model.h"
#include "Render/Shader.h"

namespace krendrr::DeferredShading
{

class DeferredShadingApp : public SDLAppBase::AppBase
{
protected:

    SDLAppBase::Camera Camera {};
    Render::Model EnvModel {};
    Render::Shader GeometryPassShader {};
    Render::Shader AmbientDirectionalLightPassShader {};
    Render::Shader PostProcessShader {};
    Render::Mesh FullscreenQuadMesh {};

    Render::Shader PointLightPassShader {};
    Render::Model PointLightUnitSphereModel {};
    std::vector<PointLight> PointLights {};

    GLuint GBufferFramebufferId {};
    GLuint GBufferColorTextureId {};
    GLuint GBufferPositionTextureId {};
    GLuint GBufferNormalTextureId {};
    GLuint GBufferMetallicTextureId {};
    GLuint GBufferDepthStencilRenderBufferId {};

    GLuint LightPassFramebufferId {};
    GLuint LightPassColorTextureId {};
    GLuint LightPassDepthStencilRenderBufferId {};

    void BeforeMainLoop() override;

    void AfterMainLoop() override;

    void OnEvent(const SDL_Event& Event) override;

    void OnKeyUp(const SDL_Event& Event) override;

    void OnKeyDown(const SDL_Event& Event) override;

    void OnMouseMove(const SDL_Event& Event) override;

    void OnMouseWheel(const SDL_Event& Event) override;

    void Render(float DeltaTime) override;

    void Update(float DeltaTime) override;

private:

    void InitializeGBuffer();
    void InitializeLightPassBuffer();

};

}
