#ifndef LIB_TOOLS_FORMATTER_H
#define LIB_TOOLS_FORMATTER_H
#pragma once

#include <vector>

namespace tools {

class Formatter
{
public:
    Formatter(int bufferSize = 512);
    Formatter& setArgs(const char *format, ...);
    const char *buffer() const;

private:
    std::vector<char> m_buffer;
    int m_bufferSize;
};


} // ns tools

#endif // LIB_TOOLS_FORMATTER_H
