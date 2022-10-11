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
        ss::SlicesScheme& slices,
        const std::string& forcedStrategySymbol)
{
    const ss::MediaType mediaType = misc::guessFileMediaType(filePath);

    if (slices.suggestedReadBufferSize == 0) {
        slices.suggestedReadBufferSize = std::min(
                    slices.dataSize,
                    misc::suggestReadBufferSizeByMediaType(mediaType, slices.blockSize));
    }

    if (!forcedStrategySymbol.empty()) {
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
            slices.suggestedReadBufferSize = seqRangeSize;
            return std::make_shared<ThreadedHashStrategy>(0, threadCountHint, seqRangeSize);
        }
    }

    if (slices.blockCount == 1) {
        return std::make_shared<SequentalHashStrategy>();
    }

    if (slices.dataSize <= slices.suggestedReadBufferSize) {
        return std::make_shared<SequentalHashStrategy>();
    }

    switch (mediaType) {
        case ss::MediaType::HDD:
            // NOTE: simple perf tests on generic notbook show no diff in speed improvemend between SSD and HDD with inc thread count
            // in both cased (SSD and HDD) speed grows with thread count grows. Need more tests
            // TODO 0: assumption that HDD can not read in mult thread, but what if it's RAID or somehting like that? Rethink!
            return std::make_shared<ThreadedHashStrategy>(0, std::thread::hardware_concurrency() / 2);
        case ss::MediaType::Memory:
        case ss::MediaType::SSD:
        case ss::MediaType::NetworkDrive:
        case ss::MediaType::Unknown:
            return std::make_shared<ThreadedHashStrategy>();
    }

    assert(false && "strategy not choosed");

    return std::make_shared<SequentalHashStrategy>();
}
