#include "slices_scheme.hpp"

#include <cassert>


ss::SlicesScheme::SlicesScheme(SizeBytes dataSize, SizeBytes blockSize)
    : dataSize(dataSize)
    , blockSize(blockSize)
{
    assert(blockSize > 0 && "block size must be positive");

    if (dataSize == 0) {
        blockCount = 1;
        needToFillUplastBlock = true;
        lastBlockRealSize = 0;
    } else {
        blockCount = dataSize / blockSize;
        lastBlockRealSize = dataSize - blockCount * blockSize;
        needToFillUplastBlock = lastBlockRealSize > 0;

        if (needToFillUplastBlock) {
            blockCount++;
        } else {
            lastBlockRealSize = blockSize;
        }
    }

    lastBlockIndex = blockCount - 1;

    assert(blockCount > 0 && "block count must be positive");
}
