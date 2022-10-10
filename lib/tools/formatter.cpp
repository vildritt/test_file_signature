#include <tools/formatter.hpp>

#include <stdarg.h>
#include <stdio.h>


tools::Formatter::Formatter(char *buffer, size_t bufferSize)
    : m_buffer(buffer), m_bufferSize(bufferSize)
{

}

tools::Formatter::Formatter(int bufferSize)
    : m_bufferSize(bufferSize)
{
    m_ownedBuffer.resize(m_bufferSize);
    m_buffer = m_ownedBuffer.data();
}


tools::Formatter &tools::Formatter::format(const char *format, ...)
{
    m_buffer[0] = 0;

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(m_buffer, m_bufferSize, format, argptr);
    va_end(argptr);

    return *this;
}


const char *tools::Formatter::c_str() const
{
    return m_buffer;
}


std::string tools::Formatter::str() const
{
    return std::string(c_str());
}
