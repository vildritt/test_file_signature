#ifndef LIB_TOOLS_HASH_MD5_H
#define LIB_TOOLS_HASH_MD5_H
#pragma once

#include <array>
#include <memory>

namespace tools {
namespace hash {
namespace md5 {


using Byte = unsigned char;
using Size = unsigned long;

struct Digest {
    static constexpr const Size kSize = 4 * 4;

    Digest();

    void clear();

    std::array<Byte, kSize> binary;
};


namespace detail {
struct HashPrivate;
} // ns detail


class Hash {
public:
    Hash();
    ~Hash() noexcept;

    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    Hash& operator=(const Hash&) = delete;
    Hash& operator=(Hash&&) = delete;

    void reinit() noexcept;
    void process(const Byte* buffer, Size size);
    Digest getDigest() const;
private:
    std::unique_ptr<detail::HashPrivate> d_ptr; // TODO 0: bad for perf - many re-creates, may be use pimpl from yandex video?
};

}}} // ns tools::hash::md5

#endif // LIB_TOOLS_HASH_MD5_H