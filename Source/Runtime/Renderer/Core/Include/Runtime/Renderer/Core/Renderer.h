#pragma once

#include <span>
#include <memory>

namespace tf
{
    class Executor;
}

namespace krendrr::Runtime::RenderApi::Core
{
    class RenderApi;
}

namespace krendrr::Runtime::Renderer::Core
{
    class SceneView;
    class Scene;

    /**
     * Renderer implements the way we render render-scene into scene view,
     * e.g. deferred, forward, forward+, etc. This is implementation of our rendering pipeline.
     *
     * It is initialized for one render scene only. You can call Shutdown and then initialize it again for another render scene.
     *
     * Render scene, scene view, etc. should not change when renderer renders it.
     *
     * Renderer may hold data between render calls, e.g. it can keep multiple GBuffers between Render calls when rendering into multiple scene views at the same time.
     */
    class Renderer
    {
    public:

        virtual bool Initialize(std::shared_ptr<RenderApi::Core::RenderApi> NewRenderApi, std::shared_ptr<tf::Executor> TfExecutor, std::shared_ptr<Core::Scene> NewScene) = 0;

        virtual bool Render(const std::span<SceneView>& SceneViews) = 0;

        virtual bool Shutdown() = 0;

        virtual ~Renderer() = default;

    };
}

