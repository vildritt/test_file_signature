#include "sequental_strategy.hpp"

#include "reader.hpp"

#include <tools/hash/digest.hpp>


void ss::SequentalHashStrategy::doHash(const Configuration &config)
{
    auto reader = config.readerfactory->create();
    auto hasher = config.hasherFactory->create();

    const bool writerAvailable = config.writer.get() != nullptr;

    const auto N = config.fileSlicesScheme.blockCount;
    for(size_t i = 0; i < N; ++i) {
        const auto digest = hasher->hash(reader->readSingleBlock(i));
        if (writerAvailable) {
            config.writer->write(digest);
        }
    }
}


std::string ss::SequentalHashStrategy::getConfigurationStringRepresentation() const
{
    return "S";
}
