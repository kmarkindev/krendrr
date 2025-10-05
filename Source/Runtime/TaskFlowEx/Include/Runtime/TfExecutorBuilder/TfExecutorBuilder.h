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
         * Sets the amount of workers. Uses std::thread::hardware_concurrency() by default.
         *
         * If set to 0, uses the default value.
         */
        Builder& SetWorkerCount(unsigned NewWorkersCount);

    private:

        bool bIsNvtxObserverAttached = true;
        unsigned WorkersCount = 0;

    };
}