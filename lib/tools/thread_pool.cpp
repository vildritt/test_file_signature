#include <tools/thread_pool.hpp>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cassert>

namespace tools {

namespace detail {

struct ThreadContext {
    std::shared_ptr<tools::ThreadPool::Context> ctx;
    std::thread thread;
};

using ThreadContextPtr = std::shared_ptr<ThreadContext>;

class ThreadPoolPrivate {
public:
    ThreadPool* q_ptr = nullptr;
    size_t poolSize;
    std::vector<ThreadContextPtr> pool;

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

    void worker(const std::shared_ptr<tools::ThreadPool::Context>& ctx)
    {
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
                job(*ctx);
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
            ThreadContextPtr tctx = std::make_shared<tools::detail::ThreadContext>();
            tctx->ctx = std::make_shared<tools::ThreadPool::Context>();
            tctx->ctx->pool = q_ptr;
            tctx->ctx->userData = nullptr;
            tctx->thread = std::thread([this, tctx]() {
                worker(tctx->ctx);
            });
            pool.push_back(tctx);
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

        for(auto& tc : pool) {
            tc->thread.join();
        }

        pool.clear();
    }
};

}} // ns tools::detail


tools::ThreadPool::~ThreadPool() noexcept
{
    try {
        stop();
    } catch (...) {
    }
}


tools::ThreadPool::ThreadPool(size_t nThreads)
    : d_ptr(new detail::ThreadPoolPrivate(this, nThreads))
{

}


void tools::ThreadPool::start()
{
    d_ptr->start();
}


void tools::ThreadPool::addJob(Job job)
{
    d_ptr->addJob(job);
}


void tools::ThreadPool::stop()
{
    d_ptr->stop();
}


size_t tools::ThreadPool::size() const
{
    return d_ptr->poolSize;
}



