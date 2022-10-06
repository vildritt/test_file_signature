#ifndef SS_DIGEST_H
#define SS_DIGEST_H
#pragma once

#include <vector>
#include <ostream>

#include "types.hpp"

namespace ss {

struct Digest {
    Digest(const ss::Byte* data = nullptr, size_t size = 0);

    std::vector<Byte> binary;

    void set(const ss::Byte* data, size_t size);
    friend std::ostream& operator<<(std::ostream &s, const Digest& d);
};


std::ostream& operator<<(std::ostream &s, const Digest& d);

} // ns ss

#endif // SS_READER_H
