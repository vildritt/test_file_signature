#include "abstract.hpp"

#include <cassert>
#include <thread>

#include "consts.hpp"
#include "misc.hpp"
#include "strategies/sequental.hpp"
#include "strategies/threaded.hpp"


void ss::HashStrategy::hash(const std::string &inFilePath, std::ostream &os, const SlicesScheme &slices)
{
    doHash(inFilePath, os, slices);
}


std::unique_ptr<ss::HashStrategy> ss::HashStrategy::chooseStrategy(
        const std::string& filePath,
        const ss::SlicesScheme& slices,
        const std::string& forcedStrategySymobl)
{
    if (forcedStrategySymobl == "S") {
        return std::make_unique<SequentalHashStrategy>();
    } else if (forcedStrategySymobl == "T") {
        return std::make_unique<ThreadedHashStrategy>();
    } else {
        if (slices.dataSize < ss::kSeqStrategyFileSizeLimitBytes) {
            return std::make_unique<SequentalHashStrategy>();
        } else {
            const ss::MediaType mediaType = misc::guessFileMediaType(filePath);
            switch (mediaType) {
                case ss::MediaType::HDD:
                    return std::make_unique<ThreadedHashStrategy>(std::thread::hardware_concurrency() / 2);
                case ss::MediaType::SSD:
                case ss::MediaType::NetworkDrive:
                case ss::MediaType::Unknown:
                    return std::make_unique<ThreadedHashStrategy>();
            }
        }
    }

    assert(false && "strategy not choosed");
}
