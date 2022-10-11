#ifndef LIB_TOOLS_FORMATTER_H
#define LIB_TOOLS_FORMATTER_H
#pragma once

#include <vector>
#include <string>

namespace tools {

/**
 * @brief Simple printf-like formatter to owned or external buffer
 */
class Formatter
{
public:
    static const int kDefaultBufferSize = 512;

    /**
     * @brief create Formatter with external (not owning) buffer
     * @param buffer - external buffer
     * @param bufferSize - size of external buffer
     */
    Formatter(char* buffer, size_t bufferSize);
    /**
     * @brief create Formatter with internal owning buffer
     * @param bufferSize - size of internal buffer
     */
    Formatter(int bufferSize = kDefaultBufferSize);

    /**
     * @brief execute formating
     * @param format - format string like in printf
     * @param ... - arguments to format
     * @return self
     */
    Formatter& format(const char *format, ...);

    /**
     * @return  ptr to formatted string buffer
     */
    const char *c_str() const;

    /**
     * @return str copy of formatted string buffer
     */
    std::string str() const;

private:
    std::vector<char> m_ownedBuffer;
    char* m_buffer;
    size_t m_bufferSize;
};


} // ns tools

#endif // LIB_TOOLS_FORMATTER_H
