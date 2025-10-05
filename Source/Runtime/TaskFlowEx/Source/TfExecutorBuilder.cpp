#include "Runtime/TfExecutorBuilder/TfExecutorBuilder.h"
#include "nvtx3/nvtx3.hpp"

namespace krendrr::Runtime::TaskFlowEx
{
    struct NvtxObserver final : public tf::ObserverInterface
    {
        void set_up(size_t num_workers) override
        {
            nvtx3::mark("NVTX Observer set up for tf::Executor");
        }

        static inline thread_local std::optional<nvtx3::scoped_range> JobRange {};

        void on_entry(tf::WorkerView wv, tf::TaskView task_view) override
        {
            JobRange.emplace(task_view.name(), nvtx3::rgb{0, 255, 0});
        }

        void on_exit(tf::WorkerView wv, tf::TaskView task_view) override
        {
            JobRange.reset();
        }
    };

    Builder Builder::Create()
    {
        return Builder {};
    }

    std::shared_ptr<tf::Executor> Builder::Build()
    {
        std::shared_ptr<tf::Executor> Executor = std::make_shared<tf::Executor>(
            WorkersCount > 0 ? WorkersCount : std::thread::hardware_concurrency()
        );

        if (bIsNvtxObserverAttached)
        {
            Executor->make_observer<NvtxObserver>();
        }

        return Executor;
    }

    Builder& Builder::AttachNvtxObserver(bool bShouldAttach)
    {
        bIsNvtxObserverAttached = bShouldAttach;

        return *this;
    }

    Builder& Builder::SetWorkerCount(unsigned NewWorkersCount)
    {
        WorkersCount = NewWorkersCount;

        return *this;
    }
}
