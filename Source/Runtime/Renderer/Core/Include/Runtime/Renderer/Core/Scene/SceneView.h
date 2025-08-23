#pragma once

#include "glad/gl.h"
#include "glm/vec4.hpp"

namespace krendrr::Runtime::Renderer::Core
{
    class SceneView
    {
    public:

        explicit SceneView(GLuint Framebuffer, glm::ivec4 Viewport);

        [[nodiscard]] GLuint GetFramebuffer() const;

        [[nodiscard]] glm::ivec4 GetViewport() const;

    private:

        GLuint Framebuffer {};
        glm::ivec4 Viewport {};

    };
}

