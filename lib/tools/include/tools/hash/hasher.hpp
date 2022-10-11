#ifndef LIB_TOOLS_HASH_HASHER_H
#define LIB_TOOLS_HASH_HASHER_H
#pragma once

#include <tools/hash/digest.hpp>

#include <string_view>
#include <memory>


namespace tools {
namespace hash {


class AbstractHasher {
public:
    AbstractHasher(const AbstractHasher&) = delete;
    AbstractHasher(AbstractHasher&&) = delete;
    AbstractHasher& operator=(const AbstractHasher&) = delete;
    AbstractHasher& operator=(AbstractHasher&&) = delete;

    AbstractHasher();
    virtual ~AbstractHasher() {}

    /**
     * @brief do get hash of single mem. block
     */
    Digest hash(const std::string_view& buffer);

    /**
     * @brief iterational big data block hashing intf
     */
    void initialize();
    void process(const std::string_view& buffer);
    Digest finalize();
private:
    virtual void doInitialize() = 0;
    virtual void doProcess(const std::string_view& buffer) = 0;
    virtual Digest doFinalize() = 0;
};


using HasherPtr = std::shared_ptr<AbstractHasher>;


class AbstractHasherFactory {
public:
    HasherPtr create();
    size_t digestSize();
private:
    virtual HasherPtr doCreate() = 0;
    virtual size_t doGetDigestSize();

    size_t m_digestSize = 0;
};


template<typename T>
class DefaultHasherFactoryImpl : public AbstractHasherFactory {
private:
    HasherPtr doCreate() override
    {
        return std::make_shared<T>();
    }
};


using HasherFactoryPtr = std::shared_ptr<AbstractHasherFactory>;


}} // ns tools::hash


#endif // LIB_TOOLS_HASH_HASHER_H
