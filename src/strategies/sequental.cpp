#include "sequental.hpp"

#include "reader.hpp"
#include "digest.hpp"
#include "consts.hpp"


void ss::SequentalHashStrategy::doHash(const std::string &inFilePath, std::ostream *os, const SlicesScheme &slices)
{
    ss::BlockReader reader(inFilePath, slices, slices.suggestedReadBufferSize);
    for(size_t i = 0; i < slices.blockCount; ++i) {
        const auto digest = ss::Digest::hashBuffer(reader.readBlock(i));
        if (os) {
            *os << digest;
        }
    }
}


std::string ss::SequentalHashStrategy::getConfString() const
{
    return "S";
}
