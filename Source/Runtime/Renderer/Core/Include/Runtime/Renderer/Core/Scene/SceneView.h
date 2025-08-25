#pragma once

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"

namespace krendrr::Runtime::Renderer::Core
{
    class SceneView
    {
    public:

        enum class ProjectionType
        {
            Orthographic,
            Perspective
        };

        struct InitParams
        {
            glm::vec3 Position {};
            glm::quat Rotation {};

            ProjectionType ProjectionType {ProjectionType::Perspective};

            float FovVertical {75.f};

            // left, right, bottom, top
            glm::vec4 OrthographicBounds {
                100.f,
                100.f,
                100.f,
                100.f,
            };

            float NearPlane {1.f};
            float FarPlane {100'000.0f};
        };

        bool Initialize(GLuint NewFramebuffer, const glm::ivec4& NewViewport, const InitParams& Params = {});

        [[nodiscard]] bool IsValid() const;

        [[nodiscard]] GLuint GetFramebuffer() const;

        [[nodiscard]] glm::ivec4 GetViewport() const;
        [[nodiscard]] glm::ivec2 GetViewportSize() const;
        [[nodiscard]] glm::mat4 GetViewMatrix() const;
        [[nodiscard]] glm::mat4 GetProjectionMatrix() const;
        [[nodiscard]] float GetNearPlane() const;
        [[nodiscard]] float GetFarPlane() const;
        [[nodiscard]] float GetFovVertical() const;
        [[nodiscard]] glm::vec3 GetPosition() const;
        [[nodiscard]] glm::quat GetRotation() const;

        void SetPosition(const glm::vec3& NewPosition);
        void SetRotation(const glm::quat& NewRotation);

        bool SetViewport(const glm::ivec4& NewViewport);

    private:

        bool bInitialized {};

        GLuint Framebuffer {};

        glm::vec3 Position {};
        glm::quat Rotation {};

        glm::ivec4 Viewport {};

        ProjectionType ProjectionType {};

        float NearPlane {};
        float FarPlane {};

        float FovVertical {};

        glm::vec4 OrthographicBounds {};

        [[nodiscard]] bool CheckValid() const;
    };
}

