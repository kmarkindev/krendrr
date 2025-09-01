#include "Runtime/ThreadPool/ThreadPool.h"
#include <condition_variable>
#define LEAN_AND_MEAN
#include <windows.h>

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
        bIsAbort.store(false, std::memory_order::relaxed);

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
        bIsAbort.store(bAbortAllJobs, std::memory_order::relaxed);

        WaitForAllJobs();

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

        Jobs.emplace(std::move(NewJob));

        WorkerUpdateCondVar.notify_one();

        return true;
    }

    void ThreadPool::WaitForAllJobs()
    {
        if (!IsInitialized())
            return;

        std::unique_lock Lock {JobsMutex};

        while (!Jobs.empty())
            WorkerFinishedCondVar.wait(Lock);
    }

    void ThreadPool::ThreadWorker()
    {
        while (true)
        {
            std::function<void()> JobToProcess {};

            {
                if (bIsAbort.load(std::memory_order_relaxed))
                    return;

                std::unique_lock Lock {JobsMutex};

                if (Jobs.empty())
                    WorkerFinishedCondVar.notify_all();

                while(Jobs.empty() && !bIsAbort.load(std::memory_order_relaxed))
                    WorkerUpdateCondVar.wait(Lock);

                if (bIsAbort.load(std::memory_order_relaxed))
                    return;

                JobToProcess = std::move(Jobs.front());
                Jobs.pop();
            }

            try
            {
                JobToProcess();
            }
            catch (...)
            {
                // TODO: log error and continue
            }
        }
    }
}
