#include "sequental.hpp"

#include "reader.hpp"
#include "digest.hpp"


void ss::SequentalHashStrategy::doHash(const std::string &inFilePath, std::ostream &os, const SlicesScheme &slices)
{
    ss::BlockReader reader(inFilePath, slices);
    for(size_t i = 0; i < slices.blockCount; ++i) {
        os << ss::Digest::hashBuffer(reader.readBlock(i));
    }
}
