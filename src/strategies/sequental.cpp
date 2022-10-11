#include "sequental.hpp"

#include "reader.hpp"

#include <tools/hash/digest.hpp>


void ss::SequentalHashStrategy::doHash(const std::string &inFilePath, const ss::DigestWriterPtr& writer, const FileSlicesScheme &slices, const tools::hash::HasherFactoryPtr &hasherFactory)
{
    ss::FileBlockReader reader(inFilePath, slices, slices.suggestedReadBufferSizeBytes);
    auto hasher = hasherFactory->create();

    const bool writerIsOk = writer.get() != nullptr;

    for(size_t i = 0; i < slices.blockCount; ++i) {
        const auto digest = hasher->hash(reader.readSingleBlock(i));
        if (writerIsOk) {
            writer->write(digest);
        }
    }
}


std::string ss::SequentalHashStrategy::getConfigurationStringRepresentation() const
{
    return "S";
}
