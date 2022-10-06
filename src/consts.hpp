#ifndef SS_CONSTS_H
#define SS_CONSTS_H
#pragma once

#include "types.hpp"

namespace ss {

static constexpr const SizeBytes kKiloBytes = 1024;
static constexpr const SizeBytes kMegaBytes = 1024 * kKiloBytes;

#ifndef _NDEBUG
static constexpr const SizeBytes kMinBlockSizeBytes = 1;
#else
static constexpr const SizeBytes kMinBlockSizeBytes = 512;
#endif
static constexpr const SizeBytes kMaxBlockSizeBytes = 10 * kMegaBytes;

static constexpr const SizeBytes kDefaultBlockSize = 1 * kMegaBytes;


} // ns ss

#endif // SS_CONSTS_H
