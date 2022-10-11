#ifndef LIB_TOOLS_HASH_MD5_H
#define LIB_TOOLS_HASH_MD5_H
#pragma once

/**
 * MD5 hasher
 */

#include <array>
#include <memory>

namespace tools {
namespace hash {
namespace md5 {


using Byte = unsigned char;
using Size = unsigned long;

/**
 * @brief MD5 digest store
 */
struct Digest {
    static constexpr const Size kSize = 4 * 4;

    Digest();

    void clear();

    std::array<Byte, kSize> binary;
};


namespace detail {
struct HashPrivate;
} // ns detail


/**
 * @brief main MD5 hasher
 */
class Hash {
public:
    /**
     * @brief create inited hasher
     */
    Hash();
    ~Hash() noexcept;

    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    Hash& operator=(const Hash&) = delete;
    Hash& operator=(Hash&&) = delete;

    /**
     * @brief retinit hasher for new data processing
     */
    void reinit() noexcept;
    /**
     * @brief process buffer data
     * Can be called sequentally several times for windowed data access
     */
    void process(const Byte* buffer, Size size);
    /**
     * @brief finaly get digest of processed data. After call hasher will be reinited
     * @return final digest
     */
    Digest getDigest() const;
private:
    // TODO 0: bad for perf - many re-creates, may be use pimpl from yandex video?
    std::unique_ptr<detail::HashPrivate> d_ptr;
};


}}} // ns tools::hash::md5


#endif // LIB_TOOLS_HASH_MD5_H
