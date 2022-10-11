#ifndef LIB_TOOLS_LOG_H
#define LIB_TOOLS_LOG_H
#pragma once

#include <string>
#include <tools/formatter.hpp>
#include <memory>

/**
 * Simple logging ability
 * - all output to std::err
 * - to use logger in some cpp:
 *      - delare logger:
 *          ST_LOGGER("prefix_name")
 *      - use log macroses:
 *          ST_[L]LOG[F](...)
 *              P - Log leval short name
 *              F - formatted / not formatted
 */

namespace tools {
namespace log {

namespace detail {
class LoggerPrivate;
}

/**
 *  @brief Very simple logger
 *  - Logs to std::cerr
 *  - prints rel time stamp, logging level, prefix, message
 *  Prefer to use macroses ST_[L]LOG[F] insted of manual work with this class
 **/
class Logger {
public:
    /**
     * @param prefix - will be used as prefix in each logging line
     */
    Logger(const std::string& prefix);
    ~Logger();

    inline int level() const noexcept {
        return m_level;
    }

    inline bool checkLevel(int level) noexcept {
        return level <= m_level;
    }

    /**
     * @brief do log message
     * @param level - logging level
     * @param msg - message
     * MT: thread-safe
     */
    void log(int level, const char* message);

    /**
     * @brief owned formatter that can be used to format strings
     * Not synched! use lock/unlock in MT
     */
    tools::Formatter &formatter();

    /**
     * @brief sync
     */
    void lock();
    void unlock();

    // lock RAII helper
    struct Locker {
        Logger* logger = nullptr;
        Locker(Logger* logger);
        ~Locker() noexcept;
    };
private:
    std::unique_ptr<detail::LoggerPrivate> d_ptr;
    int m_level;
};


void setGlobalLogLevel(int level);
void incGlobalLogLevel();
int globalLogLevel();


/// use this to decare logger in translation unit/class/namespace
#define TS_LOGGER(prefix) \
static tools::log::Logger& localLogger() {  \
    static tools::log::Logger inst(prefix); \
    return inst; \
}

/// generic log macro, prefer to use TS_[L]LOG[F] see below
#define TS_LOG(level, msg) \
do { \
    auto& l = localLogger(); \
    if (l.checkLevel(level)) { \
        l.log(level, msg); \
    } \
} while(0)

/// generic log macro, prefer to use TS_[L]LOG[F] see below
#define TS_LOGF(level, fmt, ...) \
    do { \
        auto& l = localLogger(); \
        if (l.checkLevel(level)) { \
            tools::log::Logger::Locker guard(&l); \
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

/// Shortcut macros to use logger

/// Not formatted messages (thread-safe)

#define TS_ELOG(msg)   TS_LOG(tools::log::Level::Error, msg)
#define TS_WLOG(msg)   TS_LOG(tools::log::Level::Warning, msg)
#define TS_NLOG(msg)   TS_LOG(tools::log::Level::Normal, msg)
#define TS_VLOG(msg)   TS_LOG(tools::log::Level::Verbose, msg)
#define TS_DLOG(msg)   TS_LOG(tools::log::Level::DebugL1, msg)
#define TS_D2LOG(msg)   TS_LOG(tools::log::Level::DebugL2, msg)
#define TS_D3LOG(msg)   TS_LOG(tools::log::Level::DebugL3, msg)

/// Formatted messages (thread-safe)

#define TS_ELOGF(fmt, ...)   TS_LOGF(tools::log::Level::Error, fmt, __VA_ARGS__)
#define TS_WLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Warning, fmt, __VA_ARGS__)
#define TS_NLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Normal, fmt, __VA_ARGS__)
#define TS_VLOGF(fmt, ...)   TS_LOGF(tools::log::Level::Verbose, fmt, __VA_ARGS__)
#define TS_DLOGF(fmt, ...)   TS_LOGF(tools::log::Level::DebugL1, fmt, __VA_ARGS__)
#define TS_D2LOGF(fmt, ...)   TS_LOGF(tools::log::Level::DebugL2, fmt, __VA_ARGS__)
#define TS_D3LOGF(fmt, ...)   TS_LOGF(tools::log::Level::DebugL3, fmt, __VA_ARGS__)

}} // ns

#endif // LIB_TOOLS_LOG_H
