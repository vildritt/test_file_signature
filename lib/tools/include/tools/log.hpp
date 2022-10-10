#ifndef LIB_TOOLS_HASH_LOG_H
#define LIB_TOOLS_HASH_LOG_H
#pragma once

#include <string>
#include <tools/formatter.hpp>
#include <memory>

namespace tools {
namespace log {

namespace detail {
class LoggerPrivate;
}

// very simple logger
class Logger {
public:
    Logger(const std::string& prefix);
    ~Logger();

    inline int level() const {
        return m_level;
    }
    inline bool checkLevel(int level) noexcept {
        return level <= m_level;
    }

    void log(int level, const char* msg);

    tools::Formatter &formatter();
private:
    std::unique_ptr<detail::LoggerPrivate> d_ptr;
    int m_level;
};


void setGlobalLogLevel(int level);
void incGlobalLogLevel();
int globalLogLevel();


#define TS_LOGGER(prefix) \
static tools::log::Logger& localLogger() {  \
    static tools::log::Logger inst(prefix); \
    return inst; \
}


#define TS_LOG(level, msg) \
do { \
    auto& l = localLogger(); \
    if (l.checkLevel(level)) { \
        l.log(level, msg); \
    } \
} while(0)


// TODO 1: possibly not compatible with MSVC, check!
#define TS_LOGF(level, fmt, ...) \
    do { \
        auto& l = localLogger(); \
        if (l.checkLevel(level)) { \
            l.log(level, l.formatter().format(fmt, __VA_ARGS__).c_str()); \
        } \
    } while(0)


enum Level {
    Error = 0,
    Warning = 1,
    Normal = 2,
    Verbose = 3,
    DebugL1 = 4,
    DebugL2 = 5,
    DebugL3 = 6,
};

#define TS_ELOG(msg)   TS_LOG(tools::log::Level::Error, msg)
#define TS_WLOG(msg)   TS_LOG(tools::log::Level::Warning, msg)
#define TS_NLOG(msg)   TS_LOG(tools::log::Level::Normal, msg)
#define TS_VLOG(msg)   TS_LOG(tools::log::Level::Verbose, msg)
#define TS_DLOG(msg)   TS_LOG(tools::log::Level::DebugL1, msg)
#define TS_D2LOG(msg)   TS_LOG(tools::log::Level::DebugL2, msg)
#define TS_D3LOG(msg)   TS_LOG(tools::log::Level::DebugL3, msg)

#define TS_ELOGF(fmt, ...)   TS_LOGF(tools::log::Level::Error, fmt, __VA_ARGS__)
#define TS_WLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Warning, fmt, __VA_ARGS__)
#define TS_NLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Normal, fmt, __VA_ARGS__)
#define TS_VLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Verbose, fmt, __VA_ARGS__)
#define TS_DLOGF(fmt, ...)   TS_LOGF(tools::log::Level::DebugL1, fmt, __VA_ARGS__)
#define TS_D2LOGF(fmt, ...)   TS_LOGF(tools::log::Level::DebugL2, fmt, __VA_ARGS__)
#define TS_D3LOGF(fmt, ...)   TS_LOGF(tools::log::Level::DebugL3, fmt, __VA_ARGS__)

}} // ns

#endif // LIB_TOOLS_HASH_LOG_H
