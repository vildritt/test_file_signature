#include "threaded.hpp"

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "consts.hpp"
#include "reader.hpp"
#include "digest.hpp"
#include "thread_pool.hpp"


ss::ThreadedHashStrategy::ThreadedHashStrategy(size_t poolSizeHint) :
    m_poolSizeHint(poolSizeHint)
{}

void ss::ThreadedHashStrategy::doHash(const std::string &inFilePath, std::ostream &os, const SlicesScheme &slices)
{
    ss::ThreadPool threadPool(m_poolSizeHint);
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
    std::atomic_size_t nextBlockIdToShedule(0);
    std::atomic_size_t runningTasks = 0;

    while(nextBlockResultToWrite < slices.blockCount) {
        // process results
        {
            std::unique_lock<std::mutex> guard(mutRes);
            if (!results.empty()) {

                // print all ready sequenced results
                while(!results.empty()) {
                    const auto it = results.find(nextBlockResultToWrite);
                    if (it != results.end()) {
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

        // schedule next block
        if (nextBlockIdToShedule < slices.blockCount) {
            const size_t blockIndex = nextBlockIdToShedule++;

            runningTasks++;

            threadPool.addJob([&inFilePath, &slices, blockIndex, &mutRes, &results, &cvRes, &nextBlockResultToWrite, &cvResNext, &runningTasks]() {

                // TODO 0: reuse reader with opened files in jobs!
                ss::BlockReader reader(inFilePath, slices);
                const auto digest = ss::Digest::hashBuffer(reader.readBlock(blockIndex));

                {
                    std::lock_guard<std::mutex> guard(mutRes);
                    results[blockIndex] = digest;

                    if (nextBlockResultToWrite == blockIndex) {
                        cvResNext.notify_one();
                    }
                    cvRes.notify_one();

                    runningTasks--;
                }
            });
        }
    }
}
