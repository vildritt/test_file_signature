#ifndef SS_SLICES_SCHEME_H
#define SS_SLICES_SCHEME_H
#pragma once

#include <cstddef>
#include "types.hpp"

namespace ss {

/**
 * @brief info about blocks sizes, buffers etc for segmented hashing
 */
struct FileSlicesScheme {
    /**
     * @param fileSizeBytes  - file size in bytes
     * @param blockSizeBytes - block size in bytes to eval single hash on it
     */
    FileSlicesScheme(SizeBytes fileSizeBytes, SizeBytes blockSizeBytes, SizeBytes suggestedReadBufferSizeBytes = 0);

    const SizeBytes fileSizeBytes = 0;
    const SizeBytes blockSizeBytes = 0;

    /// block count in file (including last may be not full filled block)
    size_t blockCount = 0;

    struct LastBlock {
        /// last block index. Simply blockCount - 1
        size_t index = 0;
        /// last block real size in bytes (<=blockSize)
        size_t realSizeBytes = 0;
        /// flag - is last block need to be filled with zeros
        bool needToFillUpWithZeros = true;
    } lastBlock;

    /// suggestion used to setup readed buffer size. 0 => auto choose
    SizeBytes suggestedReadBufferSizeBytes = 0;
};


} // ns ss


#endif // SS_SLICES_SCHEME_H
