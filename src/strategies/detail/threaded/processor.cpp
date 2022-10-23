#include "processor.hpp"

#include <thread>
#include <tools/log.hpp>

#include "consts.hpp"
#include "strategies/detail/threaded/reader_and_hasher_job.hpp"


TS_LOGGER("hash.threaded.ctx")


ss::detail::threaded::ThreadedHashProcessor::ThreadedHashProcessor(const ss::AbstractHashStrategy::Configuration& config,
        size_t threadPoolSizeHint,
        SizeBytes singleThreadSequentalRangeSizeBytes)
    : m_config(config)
{
    const auto effThreadPoolSizeHint = std::min(m_config.fileSlicesScheme.blockCount, threadPoolSizeHint);
    m_threadPool = std::make_unique<tools::ThreadPool>(effThreadPoolSizeHint);

    m_threadPoolSize = m_threadPool->size();
    m_readersJobsContexts.reserve(m_threadPoolSize);
    m_blocksPerThread = std::max<size_t>(
                1,
                singleThreadSequentalRangeSizeBytes / m_config.fileSlicesScheme.blockSizeBytes);

    const size_t maxResultsStoreCount = estimateMaxResultStoreCountLimit();
    m_maxResultVectorStoreCount = maxResultsStoreCount / m_blocksPerThread;

    TS_D2LOGF("init: blocks: %d", m_config.fileSlicesScheme.blockCount);
    TS_D2LOGF("init: block size: %d", m_config.fileSlicesScheme.blockSizeBytes);
    TS_D2LOGF("init: threads: %d", m_threadPoolSize);
    TS_D2LOGF("init: blocks per thread: %d", m_blocksPerThread);
    TS_D2LOGF("init: max storable resutls: %d", maxResultsStoreCount);
    TS_D2LOGF("init: max storable resutls blobs: %d", m_maxResultVectorStoreCount);
}


ss::detail::threaded::BlockReaderAndHasherPtr ss::detail::threaded::ThreadedHashProcessor::acquireBlockReaderHasher()
{
    std::lock_guard<std::mutex> guard(m_mutReadersJobsContexts);

    if (m_availableReadersJobsContexts.empty()) {
        auto res = std::make_shared<ss::detail::threaded::BlockReaderAndHasher>(
                    m_config.readerfactory->create(),
                    m_config.hasherFactory->create());
        m_readersJobsContexts.push_back(res);
        return res;
    }

    auto res = m_availableReadersJobsContexts.top();
    m_availableReadersJobsContexts.pop();
    return res;
}


void ss::detail::threaded::ThreadedHashProcessor::releaseBlockReaderHasher(const BlockReaderAndHasherPtr &ctx)
{
    std::lock_guard<std::mutex> guard(m_mutReadersJobsContexts);
    m_availableReadersJobsContexts.push(ctx);
}


void ss::detail::threaded::ThreadedHashProcessor::publicateFinishedJobResults(size_t startBlock, std::vector<tools::hash::Digest> &&digests)
{
    {
        std::lock_guard<std::mutex> guard(m_mutDigestsResults);
        TS_D3LOGF("worker: store res [%d+%d]", startBlock, digests.size());
        m_digestsResults.emplace(startBlock, std::move(digests));
    }

    if (m_nextBlockIndexToWriteResultFor == startBlock) {
        m_cvNextSequentalResultIsReady.notify_all();
    }

    {
        m_runningHasherJobsCount--;
        m_cvSomeReadAndHashJobFinished.notify_one();
    }
}


void ss::detail::threaded::ThreadedHashProcessor::checkAndWaitOnLimits()
{
    std::unique_lock<std::mutex> guard(m_mutDigestsResults);

    // to stop produce jobs due threads limit
    if (m_runningHasherJobsCount >= m_threadPoolSize) {
        m_cvSomeReadAndHashJobFinished.wait(guard);
    }

    // to stop produce jobs due memory limit
    while (m_digestsResults.size() >= m_maxResultVectorStoreCount) {
        m_cvNextSequentalResultIsReady.wait(guard);
    }
}


void ss::detail::threaded::ThreadedHashProcessor::scheduleNextReadAndHashJob()
{
    const size_t startBlock = m_nextBlockIndexToScheduleReadAndHash;
    const size_t endBlock = std::min(
                startBlock + m_blocksPerThread,
                m_config.fileSlicesScheme.blockCount);

    m_nextBlockIndexToScheduleReadAndHash = endBlock;

    m_runningHasherJobsCount++;
    TS_D3LOGF("enqueue job [%d-%d]", startBlock, endBlock - startBlock);

    m_threadPool->addJob(std::make_shared<ReaderAndHasherJob>(startBlock, endBlock, this));
}


bool ss::detail::threaded::ThreadedHashProcessor::isAllResultsDoneAndFlushed() const
{
    return m_nextBlockIndexToWriteResultFor >= m_config.fileSlicesScheme.blockCount;
}


void ss::detail::threaded::ThreadedHashProcessor::resultsWriterWorker(const ss::DigestWriterPtr& writer)
{
    std::vector<tools::hash::Digest> digests;

    while (!isAllResultsDoneAndFlushed()) {

        // try get next digests
        {
            std::unique_lock<std::mutex> guard(m_mutDigestsResults);
            while (m_digestsResults.empty()) {
                m_cvNextSequentalResultIsReady.wait(guard);
            }

            // find next result
            const auto it = m_digestsResults.find(m_nextBlockIndexToWriteResultFor.load());
            if (it == m_digestsResults.end()) {
                continue;
            }

            // get next results
            digests = std::move(it->second);
            TS_D3LOGF("writer: flush res [%d+%d]", it->first, digests.size());

            m_digestsResults.erase(it);
        }

        // do write
        if (writer) {
            for(const auto& digest : digests) {
                writer->write(digest);
            }
        }

        m_nextBlockIndexToWriteResultFor += digests.size();
    }
}


size_t ss::detail::threaded::ThreadedHashProcessor::estimateMaxResultStoreCountLimit() const
{
    const ss::SizeBytes buffersMemoryConsume =
            (m_config.fileSlicesScheme.blockSizeBytes + m_config.fileSlicesScheme.suggestedReadBufferSizeBytes)
            * m_threadPoolSize;

    const ss::SizeBytes singleHashMemConsume = m_config.hasherFactory->digestSize();

    const ss::SizeBytes maxResultStoreCountByMem = (ss::kMemoryConsumptionLimit - buffersMemoryConsume) / singleHashMemConsume;

    // NOTE: reduce by 2 cos evaluation very rude and mem limit is high
    return maxResultStoreCountByMem / 2;
}


void ss::detail::threaded::ThreadedHashProcessor::run(const ss::DigestWriterPtr& writer)
{
    m_threadPool->start();

    std::thread writerThread([this, writer]() {
        resultsWriterWorker(writer);
    });

    // main loop for producing read+hash tasks
    while (!isAllResultsDoneAndFlushed()) {

        checkAndWaitOnLimits();

        if (m_nextBlockIndexToScheduleReadAndHash < m_config.fileSlicesScheme.blockCount) {
            scheduleNextReadAndHashJob();
        }
    }

    // finalize writer thread
    m_cvNextSequentalResultIsReady.notify_all();
    writerThread.join();
}


ss::detail::threaded::ThreadedHashProcessor::BlockReaderAndHasherLocker::BlockReaderAndHasherLocker(ThreadedHashProcessor *owner)
    : owner(owner)
{
    blockReaderHasher = owner->acquireBlockReaderHasher();
}


ss::detail::threaded::ThreadedHashProcessor::BlockReaderAndHasherLocker::~BlockReaderAndHasherLocker()
{
    try {
        if (owner != nullptr) {
            owner->releaseBlockReaderHasher(blockReaderHasher);
        }
    } catch(...) {}
}


ss::detail::threaded::BlockReaderAndHasher::BlockReaderAndHasher(const ss::FileBlockReaderPtr& reader, const tools::hash::HasherPtr &hasher)
    : reader(reader)
    , hasher(hasher)
{
}


tools::hash::Digest ss::detail::threaded::BlockReaderAndHasher::readSingleBlockAndCalculateHash(size_t blockIndex)
{
    const auto bufferView = reader->readSingleBlock(blockIndex);
    return hasher->hash(bufferView);
}
