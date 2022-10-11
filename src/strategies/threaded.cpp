#include "threaded.hpp"

#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <array>
#include <condition_variable>

#include "consts.hpp"
#include "reader.hpp"
#include "digest.hpp"
#include <tools/thread_pool.hpp>


#include <tools/log.hpp>


TS_LOGGER("hash.threaded")


ss::ThreadedHashStrategy::ThreadedHashStrategy(size_t poolSizeHint, SizeBytes singleThreadSequentalRangeSize)
    : m_poolSizeHint(poolSizeHint)
    , m_singleThreadSequentalRangeSize(singleThreadSequentalRangeSize)
{
    if (m_poolSizeHint == 0) {
        m_poolSizeHint = std::thread::hardware_concurrency();
    }
}



void ss::ThreadedHashStrategy::doHash(const std::string &inFilePath, std::ostream *os, const SlicesScheme &slices)
{
    SizeBytes effSingleThreadSequentalRangeSize = m_singleThreadSequentalRangeSize > 0
            ? m_singleThreadSequentalRangeSize
            : slices.suggestedReadBufferSize;
    if (effSingleThreadSequentalRangeSize == 0) {
        effSingleThreadSequentalRangeSize = ss::kDefaultSingleThreadSequentalRangeSize;
    }

    const size_t blocksPerThread = std::max<size_t>(
                1,
                effSingleThreadSequentalRangeSize / slices.blockSize);


    tools::ThreadPool threadPool(std::min(slices.blockCount, m_poolSizeHint));

    std::mutex mutResults;
    std::condition_variable cvThreadsLimit;
    std::condition_variable cvResMemLimit;

    // TODO 2: may be use priority queue? but it's log(n). Need tests vs hash map with real data
    std::unordered_map<size_t, std::vector<ss::Digest>> results;

    size_t maxResultStoreCount = threadPool.size();

    // est limits
    {
        const ss::SizeBytes buffersMemConsume =
                (static_cast<ss::SizeBytes>(slices.blockSize) + slices.suggestedReadBufferSize)
                * threadPool.size();

        ss::SizeBytes singleHashMemConsume = 1;
        // test hasher digest size (here we do not know that digest is MD5)
        // TODO 1: improove, impr hasher intf, get digetst size from it's intf
        {
            std::array<char, 10> test{};
            singleHashMemConsume =
                    std::max<ss::SizeBytes>(
                        1,
                        ss::Digest::hashBuffer(std::string_view(test.data(), test.size())).binary.size());
        }
        ss::SizeBytes maxResultStoreCountByMem = (ss::kMemoryConsumptionLimit - buffersMemConsume) / singleHashMemConsume;
        // NOTE: reduce by 2 cos evaluation very rude and mem limit is high
        maxResultStoreCount = (maxResultStoreCount + maxResultStoreCountByMem) / 2;
    }

    const size_t maxResultVectorStoreCount = maxResultStoreCount / blocksPerThread;

    threadPool.start();

    std::atomic_size_t nextBlockResultToWrite(0);
    size_t nextBlockIdToShedule = 0;
    std::atomic_size_t runningTasksCount = 0;

    std::mutex mutReaders;
    std::vector<std::shared_ptr<ss::BlockReader>> readers;
    readers.reserve(threadPool.size());

    TS_D2LOGF("init: blocks: %d", slices.blockCount);
    TS_D2LOGF("init: block size: %d", slices.blockSize);
    TS_D2LOGF("init: threads: %d", threadPool.size());
    TS_D2LOGF("init: blocks per thread: %d", blocksPerThread);
    TS_D2LOGF("init: max storable resutls: %d", maxResultStoreCount);
    TS_D2LOGF("init: max storable resutls blobs: %d", maxResultVectorStoreCount);

    while(nextBlockResultToWrite < slices.blockCount) {
        // process ready results
        {
            std::unique_lock<std::mutex> guard(mutResults);
            if (!results.empty()) {

                // print all ready sequenced results
                while(!results.empty()) {
                    const auto it = results.find(nextBlockResultToWrite.load());
                    if (it != results.end()) {
                        auto digests = std::move(it->second);
                        const size_t startBlockId = it->first;
                        results.erase(it);

                        // free mutex during io ops
                        {
                            if (os) {
                                guard.unlock();
                                TS_D3LOGF("writer: flush res [%d+%d]", startBlockId, digests.size());
                                for(const auto& d : digests) {
                                    *os << d;
                                }
                                guard.lock();
                            }

                            nextBlockResultToWrite += digests.size();
                            if (nextBlockResultToWrite >= slices.blockCount) {
                                return;
                            }

                        }
                    } else {
                        break;
                    }
                }

                // to stop produce jobs due memory limit
                if (results.size() >= maxResultVectorStoreCount) {
                    cvResMemLimit.wait(guard);
                }
            }

            // to stop produce jobs due threads limit
            if (runningTasksCount >= threadPool.size()) {
                cvThreadsLimit.wait(guard);
            }
        }

        // schedule next blocks sequence
        if (nextBlockIdToShedule < slices.blockCount) {
            const size_t startBlock = nextBlockIdToShedule;
            const size_t endBlock = std::min(
                        startBlock + blocksPerThread,
                        slices.blockCount);
            nextBlockIdToShedule = endBlock;

            runningTasksCount++;
            TS_D3LOGF("enqueue job [%d-%d]", startBlock, endBlock - startBlock);

            threadPool.addJob([&, startBlock, endBlock](tools::ThreadPool::Context& ctx) {
                try {
                    // create reader for worker thread if not yet created
                    {
                        if (ctx.userData == nullptr) {
                            auto reader = std::make_shared<ss::BlockReader>(inFilePath, slices, slices.suggestedReadBufferSize);
                            {
                                std::lock_guard<std::mutex> guard(mutReaders);
                                readers.push_back(reader);
                            }
                            ctx.userData = reader.get();
                        }
                    }

                    // get readed assigned to worker thread
                    ss::BlockReader* reader = static_cast<ss::BlockReader*>(ctx.userData);

                    std::vector<ss::Digest> seqDigests;
                    seqDigests.reserve(endBlock - startBlock);

                    for(size_t i = startBlock; i < endBlock; ++i) {
                        //TODO 1: try to reuse hasher too, impl intf
                        seqDigests.push_back(ss::Digest::hashBuffer(reader->readBlock(i)));
                    }
                    {
                        std::lock_guard<std::mutex> guard(mutResults);
                        results.emplace(startBlock, std::move(seqDigests));
                        TS_D3LOGF("worker: store res [%d+%d]", startBlock, endBlock - startBlock);

                        if (nextBlockResultToWrite == startBlock) {
                            cvResMemLimit.notify_one();
                        }
                    }

                    {
                        cvThreadsLimit.notify_one();
                        runningTasksCount--;
                    }

                } catch (const std::exception& e) {
                    TS_ELOGF("hash job failed [%d]: %s", startBlock, e.what());
                    std::abort();
                } catch (...) {
                    TS_ELOGF("hash job failed [%d]: unkown", startBlock);
                    std::abort();
                }
            });
        }
    }
}


std::string ss::ThreadedHashStrategy::getConfString() const
{
    return tools::Formatter().format("T:%d:%lld",
                                     m_poolSizeHint,
                                     m_singleThreadSequentalRangeSize).str();
}
