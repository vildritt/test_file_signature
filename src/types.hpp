#ifndef SS_TYPES_H
#define SS_TYPES_H
#pragma once

// simple types

#include <cstdint>

namespace ss {

using SizeBytes = int64_t;
using Byte = uint8_t;

enum class MediaType {
    Unknown,
    Memory,
    SSD,
    HDD,
    NetworkDrive,
};

} // ns ss

#endif // SS_TYPES_H
