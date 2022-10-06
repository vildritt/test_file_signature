#ifndef SS_SLICES_SCHEME_H
#define SS_SLICES_SCHEME_H
#pragma once

#include <cstddef>

namespace ss {

struct SlicesScheme {
    SlicesScheme(size_t dataSize, size_t blockSize);

    size_t dataSize;
    size_t blockSize;
    size_t blockCount;
    size_t lastBlockRealSize;
    size_t lastBlockIndex;
    bool needToFillUplastBlock;
};



} // ns ss

#endif // SS_SLICES_SCHEME_H
