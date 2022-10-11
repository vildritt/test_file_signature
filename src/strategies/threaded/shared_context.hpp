#ifndef SS_STRATEGIES_THREADED_SHARED_CONTEXT_H
#define SS_STRATEGIES_THREADED_SHARED_CONTEXT_H
#pragma once

#include "reader.hpp"
#include "writers/writer.hpp"

#include <tools/hash/hasher.hpp>
#include <tools/thread_pool.hpp>

#include <stack>
#include <unordered_map>

#include <atomic>
#include <mutex>
#include <condition_variable>


namespace ss {
namespace threaded {

/**
 * @brief Reusable objects for reader jobs
 */
struct BlockReaderAndHasher {
    ss::FileBlockReader reader;
    tools::hash::HasherPtr hasher;

    BlockReaderAndHasher(
            const std::string &filePath,
            const FileSlicesScheme &slices,
            ss::SizeBytes readBufferSize,
            const tools::hash::HasherPtr& hasher);

    tools::hash::Digest readSingleBlockAndCalculateHash(size_t blockIndex);
};


using SingleBlockReaderAndHasherPtr = std::shared_ptr<BlockReaderAndHasher>;


struct SharedContext {
public:
    SharedContext(
            const std::string &inFilePath,
            const ss::FileSlicesScheme &slices,
            size_t poolSizeHint,
            ss::SizeBytes singleThreadSequentalRangeSize,
            const tools::hash::HasherFactoryPtr& hasherFactory);

    /**
     * @brief main runner
     */
    void run(const ss::DigestWriterPtr &writer);

    // job helper API
    SingleBlockReaderAndHasherPtr aqcuireBlockReaderHasher();
    void releaseBlockReaderHasher(const SingleBlockReaderAndHasherPtr& reader);

    struct SingleBlockReaderAndHasherLocker {
        SharedContext *owner = nullptr;
        SingleBlockReaderAndHasherPtr blockReaderHasher;
        SingleBlockReaderAndHasherLocker(SharedContext *owner);
        ~SingleBlockReaderAndHasherLocker();
    };

    void publicateFinishedJobResults(size_t startBlock, std::vector<tools::hash::Digest>&& digests);

private:
    std::string m_inputFilePath;
    const ss::FileSlicesScheme &m_fileSlicesScheme;
    tools::hash::HasherFactoryPtr m_hasherFactory;

    size_t m_threadPoolSize;
    size_t m_maxResultVectorStoreCount;

    size_t m_blocksPerThread;

    // results storage
    std::mutex m_mutResults;
    std::unordered_map<size_t, std::vector<tools::hash::Digest>> m_results;

    // reusabe readers/hasher
    std::mutex m_mutBlockReusableReadersAndHashers;
    std::vector<SingleBlockReaderAndHasherPtr> m_allBlockReusableReadersAndHashers;
    std::stack<SingleBlockReaderAndHasherPtr> m_availableBlockReusableReadersAndHashers;

    // limiting sync
    std::condition_variable m_cvThreadsCountLimit;
    std::condition_variable m_cvWaitableResultIsReady;

    std::unique_ptr<tools::ThreadPool> m_threadPool;

    std::atomic_size_t m_runningHasherJobsCount = 0;
    size_t m_nextBlockIdToSchedule = 0;
    std::atomic_size_t m_nextBlockResultToWrite = 0;

    /// misc procs

    void checkAndWaitOnLimits();
    void scheduleNextReadAndHashJob();
    bool isAllResultsFlushed() const;
    void resultsWriterWorker(const DigestWriterPtr &writer);
    size_t estimateMaxResultStoreCountLimit() const;
};


}} // ns ss::threaded


#endif // SS_STRATEGIES_THREADED_SHARED_CONTEXT_H
