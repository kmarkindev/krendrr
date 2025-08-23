#include "Runtime/Renderer/Core/Scene/SceneView.h"

namespace krendrr::Runtime::Renderer::Core
{
    SceneView::SceneView(GLuint Framebuffer, glm::ivec4 Viewport)
        : Framebuffer(Framebuffer), Viewport(Viewport)
    {
    }

    GLuint SceneView::GetFramebuffer() const
    {
        return Framebuffer;
    }

    glm::ivec4 SceneView::GetViewport() const
    {
        return Viewport;
    }
}
