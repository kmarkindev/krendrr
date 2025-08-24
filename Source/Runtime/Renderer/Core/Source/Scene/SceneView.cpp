#include "Runtime/Renderer/Core/Scene/SceneView.h"

namespace krendrr::Runtime::Renderer::Core
{
    bool SceneView::Initialize(GLuint NewFramebuffer, const glm::ivec4& NewViewport, const InitParams& Params)
    {
        if (IsValid())
        {
            // TODO: log error
            return false;
        }

        Framebuffer = NewFramebuffer;
        Viewport = NewViewport;

        if (Viewport.z - Viewport.x <= 0 || Viewport.w - Viewport.y <= 0)
        {
            // TODO: log error bad viewport
            return false;
        }

        Position = Params.Position;
        Rotation = Params.Rotation;

        ProjectionType = Params.ProjectionType;

        if (ProjectionType == ProjectionType::Perspective)
        {
            FovVertical = Params.FovVertical;
            if (FovVertical < 0.0 || FovVertical > 360.0)
            {
                // TODO: log error bad fov
                return false;
            }

            NearPlane = Params.NearPlane;
            FarPlane = Params.FarPlane;
        }

        if (ProjectionType == ProjectionType::Orthographic)
        {
            OrthographicBounds = Params.OrthographicBounds;
        }

        if (NearPlane >= FarPlane)
        {
            // TODO: log error bad near/far plane
            return false;
        }

        bInitialized = true;
        return true;
    }

    bool SceneView::IsValid() const
    {
        return bInitialized;
    }

    GLuint SceneView::GetFramebuffer() const
    {
        return Framebuffer;
    }

    glm::ivec4 SceneView::GetViewport() const
    {
        return Viewport;
    }

    glm::ivec2 SceneView::GetViewportSize() const
    {
        return {
            Viewport.z - Viewport.x,
            Viewport.w - Viewport.y
        };
    }

    glm::mat4 SceneView::GetViewMatrix() const
    {
        if (!CheckValid())
            return glm::mat4{1.0f};

        const glm::vec3 ForwardVector = Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 UpVector = Rotation * glm::vec3(0.0f, 1.0f, 0.0f);

        return glm::lookAt(Position, Position + ForwardVector, UpVector);
    }

    glm::mat4 SceneView::GetProjectionMatrix() const
    {
        if (!CheckValid())
            return glm::mat4{1.0f};

        switch (ProjectionType)
        {
            case ProjectionType::Orthographic:
            {
                return glm::ortho(
                    OrthographicBounds.x,
                    OrthographicBounds.y,
                    OrthographicBounds.z,
                    OrthographicBounds.w,
                    NearPlane,
                    FarPlane
                );
            }
            case ProjectionType::Perspective:
            {
                glm::ivec2 ViewportSize = GetViewportSize();
                float AspectRatio = static_cast<float>(ViewportSize.x) / static_cast<float>(ViewportSize.y);
                return glm::perspective(glm::radians(FovVertical), AspectRatio, NearPlane, FarPlane);
            }
            default:
            {
                assert(false);
                return glm::mat4{1.0f};
            }
        }
    }

    float SceneView::GetNearPlane() const
    {
        return NearPlane;
    }

    float SceneView::GetFarPlane() const
    {
        return FarPlane;
    }

    float SceneView::GetFovVertical() const
    {
        return FovVertical;
    }

    glm::vec3 SceneView::GetPosition() const
    {
        return Position;
    }

    bool SceneView::CheckValid() const
    {
        if (!IsValid())
        {
            // TODO: log error
            return false;
        }

        return true;
    }
}
