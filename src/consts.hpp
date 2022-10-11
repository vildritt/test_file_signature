#ifndef SS_CONSTS_H
#define SS_CONSTS_H
#pragma once

#include "types.hpp"

namespace ss {

static constexpr const SizeBytes kKiloBytes = 1024;
static constexpr const SizeBytes kMegaBytes = 1024 * kKiloBytes;
static constexpr const SizeBytes kGigaBytes = 1024 * kMegaBytes;

#ifndef _NDEBUG
static constexpr const SizeBytes kMinBlockSizeBytes = 1;
#else
static constexpr const SizeBytes kMinBlockSizeBytes = 512;
#endif
static constexpr const SizeBytes kMaxBlockSizeBytes = 10 * kMegaBytes;

static constexpr const SizeBytes kDefaultBlockSize = 1 * kMegaBytes;

static constexpr const SizeBytes kMaxFileSizeBytes = 128 * kGigaBytes;
static constexpr const SizeBytes kMemoryConsumptionLimit = 1 * kGigaBytes;

static constexpr const SizeBytes kDefaultSingleThreadSequentalRangeSize = 1 * ss::kMegaBytes;

static constexpr const double kPerfTestMinRunTime_s = 10.0;
static constexpr const int kPerfTestCountLimit = 1000;

static constexpr const double kFlushCachesMinPeriod_ms = 500.0;

} // ns ss

#endif // SS_CONSTS_H
