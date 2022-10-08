#include <tools/formatter.hpp>

#include <stdarg.h>
#include <stdio.h>


tools::Formatter::Formatter(int bufferSize)
    : m_bufferSize(bufferSize)
{
}


tools::Formatter &tools::Formatter::setArgs(const char *format, ...)
{
    m_buffer.resize(m_bufferSize);
    m_buffer[0] = 0;

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(m_buffer.data(), m_bufferSize, format, argptr);
    va_end(argptr);

    return *this;
}


const char *tools::Formatter::buffer() const
{
    return m_buffer.data();
}
