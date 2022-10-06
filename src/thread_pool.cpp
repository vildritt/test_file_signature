#include "thread_pool.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cassert>

namespace ss {

namespace detail {

class ThreadPoolPrivate {
public:
    ThreadPool* q_ptr = nullptr;
    size_t poolSize;
    std::vector<std::thread> pool;

    std::queue<ThreadPool::Job> jobs;
    std::mutex mutNewJob;
    std::condition_variable cvNewJob;
    std::atomic_bool runs = false;

    ThreadPoolPrivate(ThreadPool* q_ptr, size_t nThreadsHint)
        : q_ptr(q_ptr)
        , poolSize(nThreadsHint)
    {
        if (poolSize == 0) {
            poolSize = std::thread::hardware_concurrency();
            if (poolSize == 0) {
                poolSize = 1;
            }
        }
        assert(poolSize > 0 && "pool size must be > 0");
    }

    void worker() {
        while(runs) {
            ThreadPool::Job job;
            {
                std::unique_lock guard(mutNewJob);
                while(true) {
                    if (jobs.empty()) {
                        cvNewJob.wait(guard);
                        if (!runs) {
                            return;
                        }
                        if (jobs.empty()) {
                            continue;
                        }
                    }

                    job = jobs.front();
                    jobs.pop();
                    break;
                }
            }

            if (!runs) {
                return;
            }

            try {
                job();
            } catch(const std::exception& e) {
                std::cerr << "thread pool: job falied:" << e.what(); // TODO 0: use logger
                std::terminate();
            } catch(...) {
                std::cerr << "thread pool: job falied:";
                std::terminate();
            }
        }
    }

    void start()
    {
        stop();
        runs = true;
        for(size_t i = 0; i < poolSize; ++i) {
            pool.push_back(std::thread([this]() {
                worker();
            }));
        }
    }

    void addJob(ThreadPool::Job job)
    {
        {
            std::lock_guard<std::mutex> guard(mutNewJob);
            jobs.push(job);
        }
        cvNewJob.notify_one();
    }

    void stop()
    {
        runs = false;
        cvNewJob.notify_all();

        for(auto& thread : pool) {
            thread.join();
        }

        pool.clear();
    }
};

}} // ns ss::detail


ss::ThreadPool::~ThreadPool() noexcept
{
    try {
        stop();
    } catch (...) {
    }
}


ss::ThreadPool::ThreadPool(size_t nThreads)
    : d_ptr(new detail::ThreadPoolPrivate(this, nThreads))
{

}


void ss::ThreadPool::start()
{
    d_ptr->start();
}


void ss::ThreadPool::addJob(Job job)
{
    d_ptr->addJob(job);
}


void ss::ThreadPool::stop()
{
    d_ptr->stop();
}


size_t ss::ThreadPool::size() const
{
    return d_ptr->poolSize;
}



