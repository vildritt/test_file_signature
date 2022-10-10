#include "abstract.hpp"

#include <cassert>
#include <thread>

#include "consts.hpp"
#include "misc.hpp"
#include "strategies/sequental.hpp"
#include "strategies/threaded.hpp"


void ss::HashStrategy::hash(const std::string &inFilePath, std::ostream *os, const SlicesScheme &slices)
{
    doHash(inFilePath, os, slices);
}

std::string ss::HashStrategy::confString() const
{
    return getConfString();
}


std::string ss::HashStrategy::getConfString() const
{
    return std::string();
}


ss::HashStrategyPtr ss::HashStrategy::chooseStrategy(
        const std::string& filePath,
        const ss::SlicesScheme& slices,
        const std::string& forcedStrategySymbol)
{
    if (forcedStrategySymbol.size() > 0) {
        if (forcedStrategySymbol == "S") {
            return std::make_shared<SequentalHashStrategy>();
        }

        if (forcedStrategySymbol[0] == 'T') {
            const auto threadCountHint =
                    forcedStrategySymbol.size() > 1
                    ? std::stol(forcedStrategySymbol.substr(1, 1))
                    : 0;
            const auto seqRangeSize =
                    forcedStrategySymbol.size() > 2
                    ? misc::parseBlockSize(forcedStrategySymbol.substr(2))
                    : 0;
            return std::make_shared<ThreadedHashStrategy>(0, threadCountHint, seqRangeSize);
        }
    }

    if (slices.blockCount == 1) {
        return std::make_shared<SequentalHashStrategy>();
    }

    if (slices.dataSize < ss::kSeqStrategyFileSizeLimitBytes) {
        return std::make_shared<SequentalHashStrategy>();
    }

    const ss::MediaType mediaType = misc::guessFileMediaType(filePath);
    switch (mediaType) {
        case ss::MediaType::HDD:
            return std::make_shared<ThreadedHashStrategy>(0, std::thread::hardware_concurrency() / 2);
        case ss::MediaType::SSD:
        case ss::MediaType::NetworkDrive:
        case ss::MediaType::Unknown:
            return std::make_shared<ThreadedHashStrategy>();
    }

    assert(false && "strategy not choosed");
}
