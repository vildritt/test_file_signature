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

    /// context for pool worker, can be used to store and reuse some data for same thread worker
    struct Context {
        ThreadPool* pool = nullptr;
        /// can be used to reusable user data for worker
        void* userData = nullptr;
    };

    /// abstract job
    using Job = std::function<void(Context& ctx)>;

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
    void addJob(Job job);
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
