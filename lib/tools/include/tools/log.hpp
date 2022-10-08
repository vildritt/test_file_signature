#ifndef LIB_TOOLS_HASH_LOG_H
#define LIB_TOOLS_HASH_LOG_H
#pragma once

#include <string>
#include <tools/formatter.hpp>

namespace tools {
namespace log {

// very simple logger
class Logger {
public:
    Logger(const std::string& prefix);
    inline int level() const {
        return m_level;
    }
    inline bool checkLevel(int level) noexcept {
        return level <= m_level;
    }

    void log(int level, const char* msg);
private:
    std::string m_prefix;
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


// TODO 0: possibly not compatible with MSVC, recheck!
#define TS_LOGF(level, fmt, ...) \
    do { \
        auto& l = localLogger(); \
        if (l.checkLevel(level)) { \
            l.log(level, tools::Formatter().setArgs(fmt, __VA_ARGS__).buffer()); \
        } \
    } while(0)


enum Level {
    Error = 0,
    Warning = 1,
    Verbose = 2,
    Debug = 3,
};

#define TS_ELOG(msg)   TS_LOG(tools::log::Level::Error, msg)
#define TS_WLOG(msg)   TS_LOG(tools::log::Level::Warning, msg)
#define TS_VLOG(msg)   TS_LOG(tools::log::Level::Verbose, msg)
#define TS_DLOG(msg)   TS_LOG(tools::log::Level::Debug, msg)

#define TS_ELOGF(fmt, ...)   TS_LOGF(tools::log::Level::Error, fmt, __VA_ARGS__)
#define TS_WLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Warning, fmt, __VA_ARGS__)
#define TS_VLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Verbose, fmt, __VA_ARGS__)
#define TS_DLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Debug, fmt, __VA_ARGS__)

}} // ns

#endif // LIB_TOOLS_HASH_LOG_H
