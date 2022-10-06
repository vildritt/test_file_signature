#ifndef SS_SLICES_SCHEME_H
#define SS_SLICES_SCHEME_H
#pragma once

#include <cstddef>
#include "types.hpp"

namespace ss {

struct SlicesScheme {
    SlicesScheme(SizeBytes dataSize, SizeBytes blockSize);

    SizeBytes dataSize;
    size_t blockSize;
    size_t blockCount;
    size_t lastBlockRealSize;
    size_t lastBlockIndex;
    bool needToFillUplastBlock;
};



} // ns ss

#endif // SS_SLICES_SCHEME_H
