#include <tools/thread_pool.hpp>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cassert>

#include <tools/log.hpp>


TS_LOGGER("thread_pool")


namespace tools {

namespace detail {

struct ThreadContext {
    std::thread thread;
};

using ThreadContextPtr = std::shared_ptr<ThreadContext>;

class ThreadPoolPrivate {
public:
    ThreadPool* q_ptr = nullptr;
    size_t poolSize;
    std::vector<ThreadContextPtr> pool;

    std::queue<ThreadPool::JobPtr> jobs;
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

    void worker()
    {
        while(runs) {
            ThreadPool::JobPtr job;
            {
                std::unique_lock guard(mutNewJob);
                while(true) {
                    if (jobs.empty()) {
                        if (!runs) {
                            return;
                        }
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
                job->run();
            } catch(const std::exception& e) {
                TS_ELOGF("job falied:", e.what());
                std::abort();
            } catch(...) {
                TS_ELOG("job falied");
                std::abort();
            }
        }
    }

    void start()
    {
        stop();

        runs = true;
        for(size_t i = 0; i < poolSize; ++i) {
            ThreadContextPtr tctx = std::make_shared<tools::detail::ThreadContext>();
            tctx->thread = std::thread([this]() {
                worker();
            });
            pool.push_back(tctx);
        }
    }

    void addJob(const ThreadPool::JobPtr& job)
    {
        {
            std::lock_guard<std::mutex> guard(mutNewJob);
            jobs.push(job);
        }
        cvNewJob.notify_one();
    }

    void stop()
    {
        {
            std::lock_guard guard(mutNewJob);
            runs = false;
            cvNewJob.notify_all();
        }

        for(auto& tc : pool) {
            tc->thread.join();
        }

        pool.clear();
    }
};

}


} // ns tools::detail


void tools::ThreadPool::IJob::run()
{
    doRun();
}


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


void tools::ThreadPool::addJob(const JobPtr &job)
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



