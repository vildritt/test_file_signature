#include "abstract.hpp"

#include <cassert>
#include <thread>

#include "misc.hpp"
#include "strategies/sequental.hpp"
#include "strategies/threaded.hpp"


void ss::AbstractHashStrategy::hash(const std::string &inFilePath, const DigestWriterPtr &writer, const FileSlicesScheme &slices, const tools::hash::HasherFactoryPtr &hasherFactory)
{
    assert(hasherFactory.get() && "give me a haser factory");
    doHash(inFilePath, writer, slices, hasherFactory);
}


std::string ss::AbstractHashStrategy::configurationStringRepresentation() const
{
    return getConfigurationStringRepresentation();
}


std::string ss::AbstractHashStrategy::getConfigurationStringRepresentation() const
{
    return std::string();
}


ss::HashStrategyPtr ss::AbstractHashStrategy::chooseStrategy(
        const std::string& filePath,
        ss::FileSlicesScheme& slices,
        const std::string& forcedStrategySymbol)
{
    const ss::MediaType mediaType = misc::guessFileMediaType(filePath);

    if (slices.suggestedReadBufferSizeBytes == 0) {
        slices.suggestedReadBufferSizeBytes = std::min(
                    slices.fileSizeBytes,
                    misc::suggestReadBufferSizeByMediaType(mediaType, slices.blockSizeBytes));
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
            slices.suggestedReadBufferSizeBytes = seqRangeSize;
            return std::make_shared<ThreadedHashStrategy>(threadCountHint, seqRangeSize);
        }
    }

    if (slices.blockCount == 1) {
        return std::make_shared<SequentalHashStrategy>();
    }

    if (slices.fileSizeBytes <= slices.suggestedReadBufferSizeBytes) {
        return std::make_shared<SequentalHashStrategy>();
    }

    switch (mediaType) {
        case ss::MediaType::HDD:
            // NOTE: simple perf tests on generic notbook show no diff in speed improvemend between SSD and HDD with inc thread count
            // in both cased (SSD and HDD) speed grows with thread count grows. Need more tests
            // TODO 0: assumption that HDD can not read in mult thread, but what if it's RAID or somehting like that? Rethink!
            return std::make_shared<ThreadedHashStrategy>(std::thread::hardware_concurrency() / 2);
        case ss::MediaType::Memory:
        case ss::MediaType::SSD:
        case ss::MediaType::NetworkDrive:
        case ss::MediaType::Unknown:
            return std::make_shared<ThreadedHashStrategy>();
    }

    assert(false && "strategy not choosed");

    return std::make_shared<SequentalHashStrategy>();
}
