#ifndef LIB_TOOLS_HASH_MD5_HASHER_H
#define LIB_TOOLS_HASH_MD5_HASHER_H
#pragma once

#include <tools/hash/hasher.hpp>

#include <memory>


namespace tools {
namespace hash {
namespace md5 {


namespace detail {
struct HasherPrivate;
} // ns detail


class Hasher : public AbstractHasher {
public:
    Hasher();
    ~Hasher();
private:
    void doInitialize() override;
    void doProcess(const std::string_view& buffer) override;
    Digest doFinalize() override;

    std::unique_ptr<detail::HasherPrivate> d_ptr;
};


class HasherFactory : public DefaultHasherFactoryImpl<Hasher> {
private:
    size_t doGetDigestSize() override;
};


}}} // ns tools::hash


#endif // LIB_TOOLS_HASH_MD5_HASHER_H
