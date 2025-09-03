#include "Runtime/Renderer/Core/Scene/SceneView.h"

namespace krendrr::Runtime::Renderer::Core
{
    bool SceneView::Initialize(const InitParams& Params)
    {
        if (IsValid())
        {
            // TODO: log error
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

    D3D12_CPU_DESCRIPTOR_HANDLE SceneView::GetRenderTargetHandle() const
    {
        return RenderData.Handle;
    }

    D3D12_VIEWPORT SceneView::GetD3dViewport() const
    {
        return CD3DX12_VIEWPORT(Viewport.x, Viewport.y, Viewport.z, Viewport.w);
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

    glm::quat SceneView::GetRotation() const
    {
        return Rotation;
    }

    void SceneView::SetPosition(const glm::vec3& NewPosition)
    {
        Position = NewPosition;
    }

    bool SceneView::SetViewport(const glm::ivec4& NewViewport)
    {
        if (NewViewport.z - NewViewport.x <= 0 || NewViewport.w - NewViewport.y <= 0)
        {
            // TODO: log error bad viewport
            return false;
        }

        Viewport = NewViewport;

        return true;
    }

    bool SceneView::SetRenderData(const RenderViewTargetData& NewRenderData, const glm::ivec4& NewViewport)
    {
        if (SetViewport(NewViewport))
        {
            RenderData = NewRenderData;
            return true;
        }

        return false;
    }

    void SceneView::RemoveRenderData()
    {
        RenderData = {};
        Viewport = {};
    }

    void SceneView::TransitionIntoRenderTargetState(ID3D12GraphicsCommandList* CommandList) const
    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
           RenderData.RenderTarget.Get(),
           RenderData.OriginalState, D3D12_RESOURCE_STATE_RENDER_TARGET);

        CommandList->ResourceBarrier(1, &barrier);
    }

    void SceneView::TransitionIntoOriginalState(ID3D12GraphicsCommandList* CommandList) const
    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
           RenderData.RenderTarget.Get(),
           D3D12_RESOURCE_STATE_RENDER_TARGET, RenderData.OriginalState);

        CommandList->ResourceBarrier(1, &barrier);
    }

    void SceneView::SetRotation(const glm::quat& NewRotation)
    {
        Rotation = NewRotation;
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
