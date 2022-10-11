#ifndef LIB_TOOLS_THREAD_POOL_H
#define LIB_TOOLS_THREAD_POOL_H
#pragma once

#include <vector>
#include <ostream>
#include <functional>
#include <memory>

namespace tools {

namespace detail {
class ThreadPoolPrivate;
} // ns detail


/**
 * @brief Threads pool to run queued jobs
 */
class ThreadPool {
public:

    /// abstract job
    class IJob {
    protected:
        virtual void doRun() = 0;
    public:
        virtual ~IJob() {}
        void run();
    };
    using JobPtr = std::shared_ptr<IJob>;

    /**
     * @param nThreads - hint for threads count. If = 0 -> autochoose
     */
    ThreadPool(size_t nThreads = 0);
    ~ThreadPool() noexcept;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * @brief start threads pool. If already started - will restart
     */
    void start();
    /**
     * @brief enqueue job. No priorities supported
     */
    void addJob(const JobPtr& job);
    /**
     * @brief stop threads pool.
     */
    void stop();

    /**
     * @brief num of used threads
     */
    size_t size() const;
private:
    std::unique_ptr<detail::ThreadPoolPrivate> d_ptr;
};


} // ns tools


#endif // LIB_TOOLS_THREAD_POOL_H
