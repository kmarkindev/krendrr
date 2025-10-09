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
            const auto& TaskName = task_view.name();

            JobRange.emplace(TaskName.empty() ? "Unnamed Job" : TaskName, nvtx3::rgb{0, 255, 0});
        }

        void on_exit(tf::WorkerView wv, tf::TaskView task_view) override
        {
            JobRange.reset();
        }
    };

    struct Worker final : public tf::WorkerInterface
    {
        void scheduler_prologue(tf::Worker& worker) override
        {
            constexpr int PrefixSize = 18;
            constexpr int PossibleIdSize = 3;
            constexpr auto* NamePrefix = L"ThreadPool Worker ";

            std::wstring ThreadName {};
            ThreadName.reserve(PrefixSize + PossibleIdSize);

            ThreadName.append(NamePrefix);
            ThreadName.append(std::to_wstring(worker.id()));

            SetThreadDescription(worker.thread().native_handle(), ThreadName.c_str());
        }

        void scheduler_epilogue(tf::Worker& worker, std::exception_ptr ptr) override
        {

        }
    };

    Builder Builder::Create()
    {
        return Builder {};
    }

    std::shared_ptr<tf::Executor> Builder::Build()
    {
        std::shared_ptr<tf::WorkerInterface> WorkerInterface = nullptr;

        if (bAttachWorkerInterface)
            WorkerInterface = std::make_shared<Worker>();

        std::shared_ptr<tf::Executor> Executor = std::make_shared<tf::Executor>(
            WorkersCount > 0 ? WorkersCount : std::thread::hardware_concurrency(),
            std::move(WorkerInterface)
        );

        if (bIsNvtxObserverAttached)
            Executor->make_observer<NvtxObserver>();

        return Executor;
    }

    Builder& Builder::AttachNvtxObserver(bool bShouldAttach)
    {
        bIsNvtxObserverAttached = bShouldAttach;

        return *this;
    }

    Builder& Builder::AttachWorkerInterface(bool bShouldAttach)
    {
        bAttachWorkerInterface = bShouldAttach;

        return *this;
    }

    Builder& Builder::SetWorkerCount(unsigned NewWorkersCount)
    {
        WorkersCount = NewWorkersCount;

        return *this;
    }
}
