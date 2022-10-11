#include "slices_scheme.hpp"

#include <cassert>


ss::FileSlicesScheme::FileSlicesScheme(SizeBytes fileSizeBytes, SizeBytes blockSizeBytes, SizeBytes suggestedReadBufferSizeBytes)
    : fileSizeBytes(fileSizeBytes)
    , blockSizeBytes(blockSizeBytes)
    , suggestedReadBufferSizeBytes(suggestedReadBufferSizeBytes)
{
    assert(blockSizeBytes > 0 && "block size must be positive");

    if (fileSizeBytes == 0) {
        blockCount = 1;
        lastBlock.needToFillUpWithZeros = true;
        lastBlock.realSizeBytes = 0;
    } else {
        blockCount = fileSizeBytes / blockSizeBytes;
        lastBlock.realSizeBytes = fileSizeBytes - blockCount * blockSizeBytes;
        lastBlock.needToFillUpWithZeros = lastBlock.realSizeBytes > 0;

        if (lastBlock.needToFillUpWithZeros) {
            blockCount++;
        } else {
            lastBlock.realSizeBytes = blockSizeBytes;
        }
    }

    lastBlock.index = blockCount - 1;

    assert(blockCount > 0 && "block count must be positive");
}
