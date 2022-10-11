#ifndef LIB_TOOLS_HASH_DIGEST_H
#define LIB_TOOLS_HASH_DIGEST_H
#pragma once

#include <vector>
#include <ostream>
#include <string_view>

#include <tools/types.hpp>

namespace tools {
namespace hash {

/**
 * @brief Generic digest holder, independet on hashing algo digest lenght
 */
struct Digest {
    // defaults
    Digest(const Digest&) = default;
    Digest(Digest&&) = default;
    Digest& operator=(const Digest&) = default;
    Digest& operator=(Digest&&) = default;


    Digest(const tools::Byte* data = nullptr, size_t size = 0);
    Digest(const std::string_view& buffer);
    Digest& operator=(const std::string_view& buffer);


    // main digest storage, len - variable to support any hasher
    std::vector<Byte> binary;


    friend std::ostream& operator<<(std::ostream &stream, const Digest& digest);
};


std::ostream& operator<<(std::ostream &stream, const Digest& digest);


}} // ns tools::hash


#endif // LIB_TOOLS_HASH_DIGEST_H
