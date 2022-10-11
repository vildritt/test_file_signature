#include <tools/hash/digest.hpp>

#include <ios>
#include <iomanip>
#include <cassert>


tools::hash::Digest::Digest(const tools::Byte* data, size_t size)
    : Digest(std::string_view(reinterpret_cast<const char*>(data), size))
{
}


tools::hash::Digest::Digest(const std::string_view& buffer)
{
    *this = buffer;
}


tools::hash::Digest &tools::hash::Digest::operator=(const std::string_view &buffer)
{
    const auto N = buffer.data() != nullptr
            ? buffer.size()
            : 0;

    binary.resize(N);
    if (N > 0) {
        std::copy(buffer.begin(), buffer.end(), binary.begin());
    }
    return *this;
}


std::ostream &tools::hash::operator<<(std::ostream &stream, const Digest &digest)
{
    for(const auto& b : digest.binary) {
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    }

    // NOTE: avoid flushing on every short digest, also use same delimiter for all platforms ("binary compat" text files)
    stream << "\n"; // std::endl;

    return stream;
}
