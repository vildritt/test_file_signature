#include "reader_and_hasher_job.hpp"

#include <tools/log.hpp>


TS_LOGGER("hash.threaded.job")


ss::threaded::ReaderAndHasherJob::ReaderAndHasherJob(size_t startBlock, size_t endBlock, SharedContext *ctx)
    : m_startBlock(startBlock)
    , m_endBlock(endBlock)
    , m_ctx(ctx)
{
}


void ss::threaded::ReaderAndHasherJob::doRun()
{
    try {
        SharedContext::SingleBlockReaderAndHasherLocker readerHolder(m_ctx);
        execute(readerHolder.blockReaderHasher);
    } catch (const std::exception& e) {
        TS_ELOGF("hash job failed [%d]: %s", m_startBlock, e.what());
        std::abort();
    } catch (...) {
        TS_ELOGF("hash job failed [%d]: unkown", m_startBlock);
        std::abort();
    }
}


void ss::threaded::ReaderAndHasherJob::execute(const SingleBlockReaderAndHasherPtr &blockReaderHasher)
{
    std::vector<tools::hash::Digest> seqDigests;
    seqDigests.reserve(m_endBlock - m_startBlock);

    for(size_t blockIndex = m_startBlock; blockIndex < m_endBlock; ++blockIndex) {
        seqDigests.push_back(blockReaderHasher->readSingleBlockAndCalculateHash(blockIndex));
    }

    m_ctx->publicateFinishedJobResults(m_startBlock, std::move(seqDigests));
}
