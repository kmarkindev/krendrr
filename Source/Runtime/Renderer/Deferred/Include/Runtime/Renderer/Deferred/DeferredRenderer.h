#pragma once

#include "Runtime/Renderer/Core/Renderer.h"

namespace krendrr::Runtime::Renderer::Deferred
{
    class DeferredRenderer final : public Core::Renderer
    {
    public:

        void Initialize(Core::Scene* Scene) override;

        void Render(const std::span<Core::SceneView>& SceneViews) override;

        void Shutdown() override;
    };
}

