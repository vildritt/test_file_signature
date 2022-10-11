#ifndef SS_TYPES_H
#define SS_TYPES_H
#pragma once

// simple types

#include <tools/types.hpp>

namespace ss {

using SizeBytes = tools::SizeBytes;
using Byte = tools::Byte;

enum class MediaType {
    Unknown,
    Memory,
    SSD,
    HDD,
    NetworkDrive,
};

} // ns ss

#endif // SS_TYPES_H
