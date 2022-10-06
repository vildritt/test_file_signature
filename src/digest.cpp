#include "digest.hpp"

#include <ios>
#include <iomanip>


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


std::ostream &ss::operator<<(std::ostream &s, const Digest &d)
{
    for(const auto& b : d.binary) {
        s << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    }
    s << std::endl;
    return s;
}
