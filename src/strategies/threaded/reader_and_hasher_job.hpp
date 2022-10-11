#ifndef SS_STRATEGIES_THREADED_READER_AND_HASHER_JOB_H
#define SS_STRATEGIES_THREADED_READER_AND_HASHER_JOB_H
#pragma once

#include <tools/thread_pool.hpp>
#include "strategies/threaded/shared_context.hpp"


namespace ss {
namespace threaded {

class ReaderAndHasherJob : public tools::ThreadPool::IJob {
public:
    ReaderAndHasherJob(size_t startBlock, size_t endBlock, SharedContext* ctx);

protected:

    void doRun() override;

private:
    size_t m_startBlock = 0;
    size_t m_endBlock = 0;
    SharedContext* m_ctx = nullptr;

    void execute(const SingleBlockReaderAndHasherPtr& blockReaderHasher);
};


}} // ns ss


#endif // SS_STRATEGIES_THREADED_READER_AND_HASHER_JOB_H
