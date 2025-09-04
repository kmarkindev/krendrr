#pragma once

#include "d3dx12/d3dx12.h"
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

            static InitParams Default()
            {
                // https://bugs.llvm.org/show_bug.cgi?id=36684
                return {};
            }
        };

        struct RenderViewTargetData
        {
            ID3D12Resource* RenderTarget {};
            D3D12_CPU_DESCRIPTOR_HANDLE Handle {};
            D3D12_RESOURCE_STATES OriginalState {};

            RenderViewTargetData() = default;

            RenderViewTargetData(
                ID3D12Resource* RenderTarget,
                D3D12_CPU_DESCRIPTOR_HANDLE Handle,
                D3D12_RESOURCE_STATES CurrentState
            )
                : RenderTarget(RenderTarget), Handle(Handle), OriginalState(CurrentState)
            {
            }
        };

        bool Initialize(const InitParams& Params = InitParams::Default());

        [[nodiscard]] bool IsValid() const;

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetHandle() const;
        [[nodiscard]] D3D12_VIEWPORT GetD3dViewport() const;
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

        /**
         * Must be called to set up render target before rendering into the view
         */
        bool SetRenderData(const RenderViewTargetData& NewRenderData, const glm::ivec4& NewViewport);

        /**
         * Must be called before exiting the application tick if UpdateRenderData was called previously
         */
        void RemoveRenderData();

        void TransitionIntoRenderTargetState(ID3D12GraphicsCommandList* CommandList) const;

        void TransitionIntoOriginalState(ID3D12GraphicsCommandList* CommandList) const;

    private:

        bool bInitialized {};

        RenderViewTargetData RenderData {};

        glm::vec3 Position {};
        glm::quat Rotation {};

        glm::ivec4 Viewport {};

        ProjectionType ProjectionType {};

        float NearPlane {};
        float FarPlane {};

        float FovVertical {};

        glm::vec4 OrthographicBounds {};

        [[nodiscard]] bool CheckValid() const;

        bool SetViewport(const glm::ivec4& NewViewport);
    };
}

