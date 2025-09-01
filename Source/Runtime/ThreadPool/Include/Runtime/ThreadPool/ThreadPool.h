#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>

namespace krendrr::Runtime::ThreadPool
{
    class ThreadPool
    {
    public:

        ThreadPool() = default;
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) noexcept = delete;
        ThreadPool& operator=(ThreadPool&&) noexcept = delete;
        ~ThreadPool();

        /**
         * Creates worker threads
         *
         * Not thread-safe
         */
        bool Initialize(std::size_t NumberOfThreads = std::thread::hardware_concurrency());

        /**
         *
         * Not thread-safe
         */
        bool IsInitialized() const;

        /**
         * Waits for all jobs to complete, then stops all worker threads.
         *
         * If bAbortAllJobs is true, it won't process jobs left in queue.
         *
         * Not thread-safe
         */
        void Shutdown(bool bAbortAllJobs = false);

        /**
         * Pushes new job into the queue. Fails if thread pool is shutting down.
         *
         * Thread-safe
         */
        bool PushJob(std::function<void()> NewJob);

        /**
         * Waits for all jobs to complete.
         *
         * Thread-safe
         */
        void WaitForAllJobs();

    private:

        std::atomic<bool> bIsShutdown{};
        std::atomic<bool> bIsAbort{};

        std::vector<std::jthread> Threads {};

        std::condition_variable WorkerUpdateCondVar {};
        std::condition_variable WorkerFinishedCondVar {};
        std::mutex JobsMutex {};
        std::queue<std::function<void()>> Jobs {};

        void ThreadWorker();
    };
}
