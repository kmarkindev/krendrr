#include "Runtime/ThreadPool/ThreadPool.h"
#include <condition_variable>
#define LEAN_AND_MEAN
#include <windows.h>
#include "nvtx3/nvtx3.hpp"

namespace krendrr::Runtime::ThreadPool
{
    ThreadPool::~ThreadPool()
    {
        if (IsInitialized())
            Shutdown();
    }

    bool ThreadPool::Initialize(std::size_t NumberOfThreads)
    {
        if (IsInitialized())
            return false;

        bIsShutdown.store(false, std::memory_order::relaxed);
        bShouldExitWorkers.store(false, std::memory_order::relaxed);

        Threads.resize(NumberOfThreads);

        for (int ThreadIndex = 1; auto& Thread: Threads)
        {
            Thread = std::jthread{std::mem_fn(&ThreadPool::ThreadWorker), this};

            std::wstring ThreadName = L"ThreadPool Worker ";
            ThreadName.append(std::to_wstring(ThreadIndex++));
            SetThreadDescription(Thread.native_handle(), ThreadName.c_str());
        }

        return true;
    }

    bool ThreadPool::IsInitialized() const
    {
        return !Threads.empty();
    }

    void ThreadPool::Shutdown(bool bAbortAllJobs)
    {
        bIsShutdown.store(true, std::memory_order::relaxed);

        if (!bAbortAllJobs)
        {
            WaitForAllJobs();
        }
        else
        {
            // TODO: log warning "aborting N jobs during thread pool shutdown"
        }

        ExitWorkers();

        for (auto& Thread: Threads)
        {
            Thread.join();
        }

        Threads.clear();
    }

    bool ThreadPool::PushJob(std::function<void()> NewJob)
    {
        if (!IsInitialized())
            return false;

        if (bIsShutdown.load(std::memory_order_relaxed))
            return false;

        std::unique_lock Lock {JobsMutex};

        if (bIsShutdown.load(std::memory_order_relaxed))
            return false;

        nvtx3::mark("New job pushed into thread pool");

        Jobs.emplace(std::move(NewJob));

        WorkerUpdateCondVar.notify_one();

        return true;
    }

    void ThreadPool::ExitWorkers()
    {
        std::unique_lock Lock {JobsMutex};

        bShouldExitWorkers.store(true, std::memory_order::relaxed);
        WorkerUpdateCondVar.notify_all();
    }

    void ThreadPool::WaitForAllJobs()
    {
        if (!IsInitialized())
            return;

        nvtx3::scoped_range WaitAllJobsRange {"Waiting for all jobs on thread pool"};

        std::unique_lock Lock {JobsMutex};

        while (!Jobs.empty() || ActiveJobs.load(std::memory_order_relaxed) > 0)
            WorkerFinishedCondVar.wait(Lock);
    }

    void ThreadPool::ThreadWorker()
    {
        while (true)
        {
            std::function<void()> JobToProcess {};

            {
                if (bShouldExitWorkers.load(std::memory_order_relaxed))
                    return;

                std::unique_lock Lock {JobsMutex};

                while(Jobs.empty() && !bShouldExitWorkers.load(std::memory_order_relaxed))
                    WorkerUpdateCondVar.wait(Lock);

                if (bShouldExitWorkers.load(std::memory_order_relaxed))
                    return;

                JobToProcess = std::move(Jobs.front());
                Jobs.pop();

                ActiveJobs.fetch_add(1, std::memory_order::relaxed);
            }

            try
            {
                nvtx3::scoped_range JobProcessRange {"Processing Job"};
                JobToProcess();
            }
            catch (...)
            {
                // TODO: log error and continue
            }

            {
                std::unique_lock Lock {JobsMutex};

                ActiveJobs.fetch_sub(1, std::memory_order::relaxed);

                // Notify waiter AFTER job was processed
                if (ActiveJobs.load(std::memory_order_relaxed) == 0 && Jobs.empty())
                    WorkerFinishedCondVar.notify_all();
            }

        }
    }
}
