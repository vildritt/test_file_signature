#ifndef SS_SLICES_SCHEME_H
#define SS_SLICES_SCHEME_H
#pragma once

#include <cstddef>
#include "types.hpp"

namespace ss {

struct SlicesScheme {
    SlicesScheme(SizeBytes dataSize, SizeBytes blockSize);

    SizeBytes dataSize = 0;
    size_t blockSize = 0;
    size_t blockCount = 0;
    size_t lastBlockRealSize = 0;
    size_t lastBlockIndex = 0;
    SizeBytes suggestedReadBufferSize = 0;
    bool needToFillUplastBlock;
};



} // ns ss

#endif // SS_SLICES_SCHEME_H
