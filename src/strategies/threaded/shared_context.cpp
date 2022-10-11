#include "shared_context.hpp"

#include "strategies/threaded/reader_and_hasher_job.hpp"
#include "consts.hpp"

#include <tools/log.hpp>
#include <thread>


TS_LOGGER("hash.threaded.ctx")


ss::threaded::SharedContext::SharedContext(
        const std::string &inFilePath,
        const FileSlicesScheme &slices,
        size_t poolSizeHint,
        SizeBytes singleThreadSequentalRangeSize,
        const tools::hash::HasherFactoryPtr &hasherFactory)
    : m_inputFilePath(inFilePath)
    , m_fileSlicesScheme(slices)
    , m_hasherFactory(hasherFactory)
{
    m_blocksPerThread = std::max<size_t>(
                1,
                singleThreadSequentalRangeSize / slices.blockSizeBytes);

    m_threadPool = std::make_unique<tools::ThreadPool>(std::min(slices.blockCount, poolSizeHint));

    m_threadPoolSize = m_threadPool->size();
    m_allBlockReusableReadersAndHashers.reserve(m_threadPoolSize);

    const size_t maxResultStoreCount = estimateMaxResultStoreCountLimit();
    m_maxResultVectorStoreCount = maxResultStoreCount / m_blocksPerThread;

    TS_D2LOGF("init: blocks: %d", slices.blockCount);
    TS_D2LOGF("init: block size: %d", slices.blockSizeBytes);
    TS_D2LOGF("init: threads: %d", m_threadPoolSize);
    TS_D2LOGF("init: blocks per thread: %d", m_blocksPerThread);
    TS_D2LOGF("init: max storable resutls: %d", maxResultStoreCount);
    TS_D2LOGF("init: max storable resutls blobs: %d", m_maxResultVectorStoreCount);

}


ss::threaded::SingleBlockReaderAndHasherPtr ss::threaded::SharedContext::aqcuireBlockReaderHasher()
{
    std::lock_guard<std::mutex> guard(m_mutBlockReusableReadersAndHashers);

    if (m_availableBlockReusableReadersAndHashers.empty()) {
        auto res = std::make_shared<ss::threaded::BlockReaderAndHasher>(
                    m_inputFilePath, m_fileSlicesScheme, m_fileSlicesScheme.suggestedReadBufferSizeBytes,
                    m_hasherFactory->create());
        m_allBlockReusableReadersAndHashers.push_back(res);
        return res;
    }

    auto res = m_availableBlockReusableReadersAndHashers.top();
    m_availableBlockReusableReadersAndHashers.pop();
    return res;
}


void ss::threaded::SharedContext::releaseBlockReaderHasher(const SingleBlockReaderAndHasherPtr &reader)
{
    std::lock_guard<std::mutex> guard(m_mutBlockReusableReadersAndHashers);
    m_availableBlockReusableReadersAndHashers.push(reader);
}


void ss::threaded::SharedContext::publicateFinishedJobResults(size_t startBlock, std::vector<tools::hash::Digest> &&digests)
{
    {
        std::lock_guard<std::mutex> guard(m_mutResults);
        TS_D3LOGF("worker: store res [%d+%d]", startBlock, digests.size());
        m_results.emplace(startBlock, std::move(digests));
    }

    if (m_nextBlockResultToWrite == startBlock) {
        m_cvWaitableResultIsReady.notify_all();
    }

    {
        m_cvThreadsCountLimit.notify_one();
        m_runningHasherJobsCount--;
    }
}


void ss::threaded::SharedContext::checkAndWaitOnLimits()
{
    std::unique_lock<std::mutex> guard(m_mutResults);

    // to stop produce jobs due threads limit
    if (m_runningHasherJobsCount >= m_threadPoolSize) {
        m_cvThreadsCountLimit.wait(guard);
    }

    // to stop produce jobs due memory limit
    while (m_results.size() >= m_maxResultVectorStoreCount) {
        m_cvWaitableResultIsReady.wait(guard);
    }
}


void ss::threaded::SharedContext::scheduleNextReadAndHashJob()
{
    const size_t startBlock = m_nextBlockIdToSchedule;
    const size_t endBlock = std::min(
                startBlock + m_blocksPerThread,
                m_fileSlicesScheme.blockCount);

    m_nextBlockIdToSchedule = endBlock;

    m_runningHasherJobsCount++;
    TS_D3LOGF("enqueue job [%d-%d]", startBlock, endBlock - startBlock);

    m_threadPool->addJob(std::make_shared<ReaderAndHasherJob>(startBlock, endBlock, this));
}


bool ss::threaded::SharedContext::isAllResultsFlushed() const
{
    return m_nextBlockResultToWrite >= m_fileSlicesScheme.blockCount;
}


void ss::threaded::SharedContext::resultsWriterWorker(const ss::DigestWriterPtr& writer)
{
    std::vector<tools::hash::Digest> digests;

    while (!isAllResultsFlushed()) {

        // try get next digests
        {
            std::unique_lock<std::mutex> guard(m_mutResults);
            while (m_results.empty()) {
                m_cvWaitableResultIsReady.wait(guard);
            }

            // find next result
            const auto it = m_results.find(m_nextBlockResultToWrite.load());
            if (it == m_results.end()) {
                continue;
            }

            // get next results
            digests = std::move(it->second);
            TS_D3LOGF("writer: flush res [%d+%d]", it->first, digests.size());

            m_results.erase(it);
        }

        // do write
        if (writer) {
            for(const auto& digest : digests) {
                writer->write(digest);
            }
        }

        m_nextBlockResultToWrite += digests.size();
    }
}


size_t ss::threaded::SharedContext::estimateMaxResultStoreCountLimit() const
{
    const ss::SizeBytes buffersMemConsume =
            (static_cast<ss::SizeBytes>(m_fileSlicesScheme.blockSizeBytes) + m_fileSlicesScheme.suggestedReadBufferSizeBytes)
            * m_threadPoolSize;

    const ss::SizeBytes singleHashMemConsume = m_hasherFactory->digestSize();

    const ss::SizeBytes maxResultStoreCountByMem = (ss::kMemoryConsumptionLimit - buffersMemConsume) / singleHashMemConsume;

    // NOTE: reduce by 2 cos evaluation very rude and mem limit is high
    return (m_threadPoolSize + maxResultStoreCountByMem) / 2;
}


void ss::threaded::SharedContext::run(const ss::DigestWriterPtr& writer)
{
    m_threadPool->start();

    std::thread writerThread([this, writer]() {
        resultsWriterWorker(writer);
    });

    while (!isAllResultsFlushed()) {

        checkAndWaitOnLimits();

        if (m_nextBlockIdToSchedule < m_fileSlicesScheme.blockCount) {
            scheduleNextReadAndHashJob();
        }
    }

    // finalize writer thread
    m_cvWaitableResultIsReady.notify_all();
    writerThread.join();
}


ss::threaded::SharedContext::SingleBlockReaderAndHasherLocker::SingleBlockReaderAndHasherLocker(SharedContext *owner)
    : owner(owner)
{
    blockReaderHasher = owner->aqcuireBlockReaderHasher();
}


ss::threaded::SharedContext::SingleBlockReaderAndHasherLocker::~SingleBlockReaderAndHasherLocker()
{
    try {
        if (owner != nullptr) {
            owner->releaseBlockReaderHasher(blockReaderHasher);
        }
    } catch(...) {}
}


ss::threaded::BlockReaderAndHasher::BlockReaderAndHasher(const std::string &filePath, const FileSlicesScheme &slices, SizeBytes readBufferSize, const tools::hash::HasherPtr &hasher)
    : reader(filePath, slices, readBufferSize)
    , hasher(hasher)
{
}


tools::hash::Digest ss::threaded::BlockReaderAndHasher::readSingleBlockAndCalculateHash(size_t blockIndex)
{
    const auto bufferView = reader.readSingleBlock(blockIndex);
    return hasher->hash(bufferView);
}
