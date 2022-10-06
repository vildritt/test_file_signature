#ifndef SS_DIGEST_H
#define SS_DIGEST_H
#pragma once

#include <vector>
#include <ostream>
#include <string_view>

#include "types.hpp"

namespace ss {

struct Digest {
    Digest(const ss::Byte* data = nullptr, size_t size = 0);

    Digest(const Digest&) = default;
    Digest(Digest&&) = default;
    Digest& operator=(const Digest&) = default;
    Digest& operator=(Digest&&) = default;

    std::vector<Byte> binary;

    void set(const ss::Byte* data, size_t size);
    friend std::ostream& operator<<(std::ostream &s, const Digest& d);

    // TODO 1: make possible to replace hashe
    static ss::Digest hashBuffer(const std::string_view& buffer);
};


std::ostream& operator<<(std::ostream &s, const Digest& d);



} // ns ss

#endif // SS_READER_H
