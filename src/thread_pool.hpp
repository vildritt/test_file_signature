#ifndef SS_THREAD_POOL_H
#define SS_THREAD_POOL_H
#pragma once

#include <vector>
#include <ostream>
#include <functional>
#include <memory>

#include "types.hpp"

namespace ss {

namespace detail {
class ThreadPoolPrivate;
} // ns detail

class ThreadPool {
public:

    struct Context {
        ThreadPool* pool = nullptr;
        void* userData = nullptr;
    };

    using Job = std::function<void(Context& ctx)>;

    ThreadPool(size_t nThreads = 0);
    ~ThreadPool() noexcept;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void start();
    void addJob(Job job);
    void stop();

    size_t size() const;
private:
    std::unique_ptr<detail::ThreadPoolPrivate> d_ptr;
};

} // ns ss

#endif // SS_THREAD_POOL_H
