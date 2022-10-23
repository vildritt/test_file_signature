#ifndef SS_STRATEGIES_THREADED_PROCESSOR_H
#define SS_STRATEGIES_THREADED_PROCESSOR_H
#pragma once

#include "reader.hpp"
#include "writers/abstract_writer.hpp"
#include "strategies/abstract_strategy.hpp"

#include <tools/hash/abstract_hasher.hpp>
#include <tools/thread_pool.hpp>

#include <stack>
#include <unordered_map>

#include <atomic>
#include <mutex>
#include <condition_variable>


namespace ss {
namespace detail {
namespace threaded {

/**
 * @brief Reusable reader/hasher context
 */
struct BlockReaderAndHasher {
    ss::FileBlockReaderPtr reader;
    tools::hash::HasherPtr hasher;

    BlockReaderAndHasher(const FileBlockReaderPtr &reader,
            const tools::hash::HasherPtr& hasher);

    tools::hash::Digest readSingleBlockAndCalculateHash(size_t blockIndex);
};


using BlockReaderAndHasherPtr = std::shared_ptr<BlockReaderAndHasher>;


/**
 * @brief Contains shared state and do a orcestration
 */
struct ThreadedHashProcessor {
public:

    /**
     * @brief ThreadedHashStrategyPrivate
     * @param config - config from strategy (factories, slice scheme etc)
     * @param poolSizeHint
     * @param singleThreadSequentalRangeSize
     */
    ThreadedHashProcessor(const ss::AbstractHashStrategy::Configuration& config,
            size_t threadPoolSizeHint,
            ss::SizeBytes singleThreadSequentalRangeSizeBytes);

    /**
     * @brief main runner
     */
    void run(const ss::DigestWriterPtr &writer);

    // reader job helper API to get reusable context (reader, hasher), publicate results

    BlockReaderAndHasherPtr acquireBlockReaderHasher();
    void releaseBlockReaderHasher(const BlockReaderAndHasherPtr& ctx);

    struct BlockReaderAndHasherLocker {
        ThreadedHashProcessor *owner = nullptr;
        BlockReaderAndHasherPtr blockReaderHasher;
        BlockReaderAndHasherLocker(ThreadedHashProcessor *owner);
        ~BlockReaderAndHasherLocker();
    };

    void publicateFinishedJobResults(size_t startBlock, std::vector<tools::hash::Digest>&& digests);

private:
    ss::AbstractHashStrategy::Configuration m_config;

    std::unique_ptr<tools::ThreadPool> m_threadPool;

    size_t m_threadPoolSize;
    size_t m_blocksPerThread;
    size_t m_maxResultVectorStoreCount;

    // results storage
    std::mutex m_mutDigestsResults;
    std::unordered_map<size_t, std::vector<tools::hash::Digest>> m_digestsResults;

    // reusabe readers/hasher contexts
    std::mutex m_mutReadersJobsContexts;
    std::vector<BlockReaderAndHasherPtr> m_readersJobsContexts;         ///< all
    std::stack<BlockReaderAndHasherPtr> m_availableReadersJobsContexts; ///< free to use

    // limiting sync
    std::condition_variable m_cvSomeReadAndHashJobFinished;
    std::condition_variable m_cvNextSequentalResultIsReady;

    // runtime counters
    std::atomic_size_t m_runningHasherJobsCount = 0;
    size_t m_nextBlockIndexToScheduleReadAndHash = 0;
    std::atomic_size_t m_nextBlockIndexToWriteResultFor = 0;

    ///

    bool isAllResultsDoneAndFlushed() const;
    size_t estimateMaxResultStoreCountLimit() const;

    void checkAndWaitOnLimits();
    void scheduleNextReadAndHashJob();
    void resultsWriterWorker(const DigestWriterPtr &writer);
};


}}} // ns ss::detail::threaded


#endif // SS_STRATEGIES_THREADED_PROCESSOR_H
