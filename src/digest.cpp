#include "digest.hpp"

#include <ios>
#include <iomanip>
#include <cassert>

#include <tools/hash/md5.hpp>


ss::Digest::Digest(const Byte *data, size_t size)
{
    if (data != nullptr && size > 0) {
        set(data, size);
    }
}


void ss::Digest::set(const Byte *data, size_t size)
{
    binary.resize(size);
    std::copy(data, data + size, binary.begin());
}


ss::Digest ss::Digest::hashBuffer(const std::string_view &buffer)
{
    assert(!buffer.empty() && "buffer must contain some data");

    tools::hash::md5::Hash hasher;
    hasher.process(
                reinterpret_cast<const tools::hash::md5::Byte*>(buffer.data()),
                static_cast<tools::hash::md5::Size>(buffer.size()));
    const auto digest = hasher.getDigest();
    return ss::Digest(digest.binary.data(), tools::hash::md5::Digest::kSize);
}


std::ostream &ss::operator<<(std::ostream &s, const Digest &d)
{
    for(const auto& b : d.binary) {
        s << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    }

    // NOTE: avoid flushinf on every short digest, also use same delimiter for all platforms (binary compat text files)
    // TODO 1: rethink
    s << "\n"; // std::endl;

    return s;
}
