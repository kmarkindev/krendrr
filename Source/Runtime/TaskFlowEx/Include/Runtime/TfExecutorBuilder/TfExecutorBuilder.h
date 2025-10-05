#pragma once

#include <taskflow/taskflow.hpp>

namespace krendrr::Runtime::TaskFlowEx
{

    class Builder
    {
    public:

        /**
         * Create builder instance
         */
        static Builder Create();

        /**
         * Build tf::Executor instance based on builder configuration
         */
        std::shared_ptr<tf::Executor> Build();

        /**
         * Attaches NVTX3 observer, so all jobs are visible in NVIDIA Nsight toolset.
         *
         * Attached by default.
         */
        Builder& AttachNvtxObserver(bool bShouldAttach = true);

        /**
         * Sets the amount of workers. Uses std::thread::hardware_concurrency() - 1 by default.
         *
         * Note: we subtract 1 because we usually have a main thread that pushes top level jobs and then coruns them,
         * so it is not part of workers set, but becomes one of them when needed.
         *
         * If set to 0, uses the default value.
         */
        Builder& SetWorkerCount(unsigned NewWorkersCount);

    private:

        bool bIsNvtxObserverAttached = true;
        unsigned WorkersCount = 0;

    };
}