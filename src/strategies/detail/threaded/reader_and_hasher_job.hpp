#ifndef SS_STRATEGIES_THREADED_READER_AND_HASHER_JOB_H
#define SS_STRATEGIES_THREADED_READER_AND_HASHER_JOB_H
#pragma once

#include <tools/thread_pool.hpp>
#include "strategies/detail/threaded/processor.hpp"


namespace ss {
namespace detail {
namespace threaded {

class ReaderAndHasherJob : public tools::ThreadPool::IJob {
public:
    ReaderAndHasherJob(size_t startBlock, size_t endBlock, ThreadedHashProcessor* ctx);

protected:

    void doRun() override;

private:
    size_t m_startBlock = 0;
    size_t m_endBlock = 0;
    ThreadedHashProcessor* m_ctx = nullptr;

    void execute(const BlockReaderAndHasherPtr& blockReaderHasher);
};


}}} // ns ss::detail::threaded


#endif // SS_STRATEGIES_THREADED_READER_AND_HASHER_JOB_H
