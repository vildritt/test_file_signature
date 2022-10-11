#ifndef SS_SLICES_SCHEME_H
#define SS_SLICES_SCHEME_H
#pragma once

#include <cstddef>
#include "types.hpp"

namespace ss {

/**
 * @brief info about blocks sizes, buffers etc for segmented hashing
 */
struct SlicesScheme {
    /**
     * @param dataSize - file size in bytes
     * @param blockSize - hashable block size in bytes
     */
    SlicesScheme(SizeBytes dataSize, SizeBytes blockSize);

    /// in file size in bytes
    SizeBytes dataSize = 0;
    /// block size in bytes
    size_t blockSize = 0;
    /// block count in file (including last may be not full filled block)
    size_t blockCount = 0;
    /// last block real size in bytes (<=blockSize)
    size_t lastBlockRealSize = 0;
    /// lasst block index. Simply blockCount - 1
    size_t lastBlockIndex = 0;

    /// suggestion used to setup readed buffer size. 0 -> auto choose
    SizeBytes suggestedReadBufferSize = 0;
    /// flag - is last block need to be filled with zeros
    bool needToFillUplastBlock;
};



} // ns ss

#endif // SS_SLICES_SCHEME_H
