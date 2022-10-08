#include "threaded.hpp"

#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "consts.hpp"
#include "reader.hpp"
#include "digest.hpp"
#include <tools/thread_pool.hpp>


#include <tools/log.hpp>


TS_LOGGER("hash.threaded")


ss::ThreadedHashStrategy::ThreadedHashStrategy(size_t blocksPerThread, size_t poolSizeHint)
    : m_poolSizeHint(poolSizeHint)
    , m_blocksPerThread(blocksPerThread)
{}

void ss::ThreadedHashStrategy::doHash(const std::string &inFilePath, std::ostream &os, const SlicesScheme &slices)
{
    if (m_poolSizeHint == 0) {
        m_poolSizeHint = std::thread::hardware_concurrency();
    }

    size_t effBlocksPerThread = m_blocksPerThread;
    if (effBlocksPerThread == 0) {
        const size_t kSingleReadedBlobSize = 1 * ss::kMegaBytes;
        effBlocksPerThread = kSingleReadedBlobSize / slices.blockSize;
    }


    tools::ThreadPool threadPool(std::min(slices.blockCount, m_poolSizeHint));
    std::mutex mutRes;
    std::condition_variable cvRes;
    std::condition_variable cvResNext;

    // TODO 1: may be use priority queue
    std::unordered_map<int, ss::Digest> results;
    size_t maxResultStoreCount = threadPool.size();

    {

        const ss::SizeBytes buffersMemConsume = static_cast<ss::SizeBytes>(slices.blockSize) * threadPool.size();
        ss::SizeBytes singleHashMemConsume = 1;
        // test hasher digest size
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

    threadPool.start();
    std::atomic_size_t nextBlockResultToWrite(0);
    size_t nextBlockIdToShedule = 0;
    std::atomic_size_t runningTasks = 0;

    std::mutex mutReaders;
    std::vector<std::shared_ptr<ss::BlockReader>> readers;

    TS_DLOGF("blocks: %d", slices.blockCount);
    TS_DLOGF("block size: %d", slices.blockSize);
    TS_DLOGF("threads: %d", threadPool.size());
    TS_DLOGF("blocks per thread: %d", effBlocksPerThread);
    TS_DLOGF("max storable resutls: %d", maxResultStoreCount);

    while(nextBlockResultToWrite < slices.blockCount) {
        // process results
        {
            std::unique_lock<std::mutex> guard(mutRes);
            if (!results.empty()) {

                // print all ready sequenced results
                while(!results.empty()) {
                    const auto it = results.find(nextBlockResultToWrite);
                    if (it != results.end()) {
                        TS_DLOGF("flush res [%d]", it->first);
                        os << it->second;
                        results.erase(it);
                        nextBlockResultToWrite++;
                        if (nextBlockResultToWrite >= slices.blockCount) {
                            return;
                        }
                    } else {
                        break;
                    }
                }

                if (results.size() >= maxResultStoreCount) {
                    cvResNext.wait(guard);
                }
            }
            if (runningTasks >= threadPool.size()) {
                cvRes.wait(guard);
            }
        }

        // not really useful misc limit, TODO 1: review
        effBlocksPerThread = std::min(effBlocksPerThread, maxResultStoreCount / threadPool.size());

        // schedule next block
        if (nextBlockIdToShedule < slices.blockCount) {
            const size_t startBlock = nextBlockIdToShedule;
            const size_t endBlock = std::min(
                        startBlock + effBlocksPerThread,
                        slices.blockCount);
            nextBlockIdToShedule = endBlock;

            runningTasks++;
            TS_DLOGF("enqueue job [%d-%d]", startBlock, endBlock - startBlock);

            threadPool.addJob([&, startBlock, endBlock]
            (tools::ThreadPool::Context& ctx) {
                try {
                    // create reader for worker thread if not yet created
                    {
                        if (ctx.userData == nullptr) {
                            auto reader = std::make_shared<ss::BlockReader>(inFilePath, slices);
                            {
                                std::lock_guard<std::mutex> guard(mutReaders);
                                readers.push_back(reader);
                            }
                            ctx.userData = reader.get();
                        }
                    }

                    ss::BlockReader* reader = static_cast<ss::BlockReader*>(ctx.userData);

                    for(size_t i = startBlock; i < endBlock; ++i) {

                        //TODO 1: try to reuse hasher too, impl intf
                        const auto digest = ss::Digest::hashBuffer(reader->readBlock(i));

                        {
                            // TODO 0: may be also check we need to check result not overfilled and sleep on cv
                            // TODO 0: may be for too small block size also do not report results immediately, but collect some and then push
                            std::lock_guard<std::mutex> guard(mutRes);
                            results[i] = digest;
                            TS_DLOGF("store res [%d]", i);

                            if (nextBlockResultToWrite == i) {
                                cvResNext.notify_one(); // TODO 0: may be use change logic to next res is fried? or
                            }
                        }
                    }

                    {
                        std::lock_guard<std::mutex> guard(mutRes);
                        cvRes.notify_one();
                        runningTasks--;
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
