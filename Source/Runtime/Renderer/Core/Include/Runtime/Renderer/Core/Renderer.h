#pragma once

#include <functional>
#include <span>
#include <memory>

namespace tf
{
    class Executor;
    class FlowBuilder;
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
     * Renderer implements the way we render scene into scene view,
     * e.g. deferred, forward, forward+, etc. This is implementation of our rendering pipeline.
     *
     * Scene and views should not change when renderer renders it.
     * But, renderer is allowed to save data into scene's buffers,
     * e.g. saving a shadow map into point light object.
     *
     * Renderer may hold data between render calls, e.g. it can keep multiple
     * GBuffers between render calls when rendering into multiple scene views at the same time.
     */
    class Renderer
    {
    public:

        virtual bool Initialize(std::shared_ptr<RenderApi::Core::RenderApi> NewRenderApi, std::shared_ptr<tf::Executor> NewTfExecutor) = 0;

        /**
         * Use Taskflow to create task graphs (simplified render graph) and run your passes in parallel when needed.
         * Provided taskflow is usually composed into some high-level taskflow.
         *
         * ErrorStop can be called from Taskflow tasks to actually request a stop of taskflow execution in case of an error.
         */
        virtual bool Render(const Scene* Scene, const std::span<SceneView>& SceneViews, tf::FlowBuilder& FlowBuilder) = 0;

        virtual bool Shutdown() = 0;

        virtual ~Renderer() = default;

    };
}

