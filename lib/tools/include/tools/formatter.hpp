#ifndef LIB_TOOLS_FORMATTER_H
#define LIB_TOOLS_FORMATTER_H
#pragma once

#include <vector>
#include <string>

namespace tools {

class Formatter
{
public:
    static const int kDefaultBufferSize = 512;

    Formatter(char* buffer, size_t bufferSize);
    Formatter(int bufferSize = kDefaultBufferSize);

    Formatter& format(const char *format, ...);
    const char *c_str() const;
    std::string str() const;

private:
    std::vector<char> m_ownedBuffer;
    char* m_buffer;
    size_t m_bufferSize;
};


} // ns tools

#endif // LIB_TOOLS_FORMATTER_H
