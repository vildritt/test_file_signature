#include <tools/log.hpp>

#include <atomic>
#include <iostream>
#include <iomanip>
#include <mutex>

#include <tools/timer.hpp>

namespace  {

std::atomic_int gLogLevel(tools::log::Level::Normal);

const char* levelToName(int level)
{
    switch(level) {
    case tools::log::Level::Error:
        return "ERRO";
    case tools::log::Level::Warning:
        return "WARN";
    case tools::log::Level::Verbose:
        return "VERB";
    case tools::log::Level::DebugL1:
        return "DEBG";
    case tools::log::Level::DebugL2:
        return "DBG2";
    case tools::log::Level::DebugL3:
        return "DBG3";
    default:
        return "UNKN";
    }
}

tools::Timer gAppStartTimer;

}

namespace tools {
namespace log {
namespace detail {

class LoggerPrivate {
public:
    Logger* q_ptr = nullptr;

    std::string m_prefix;
    std::mutex m_mutex;
    tools::Formatter m_formatter;

    LoggerPrivate(Logger* q_ptr)
        : q_ptr(q_ptr)
    {
    }
};

}}}


tools::log::Logger::Logger(const std::string &prefix)
    : d_ptr(new detail::LoggerPrivate(this))
    , m_level(gLogLevel)
{
    d_ptr->m_prefix = prefix;
}


tools::log::Logger::~Logger()
{
    // for pimp
}


void tools::log::Logger::log(int level, const char *msg)
{
    if (!checkLevel(level)) {
        return;
    }

    std::lock_guard<std::mutex> guard(d_ptr->m_mutex);

    const auto t_s = gAppStartTimer.elapsed_s();
    std::cerr << "[" << std::setw(10) << std::fixed << std::setprecision(3) << t_s << std::setw(0) << "]"
              << "[" << levelToName(level) << "]"
              << " ["  << d_ptr->m_prefix << "]: "
              << msg
              << std::endl;
}


tools::Formatter &tools::log::Logger::formatter()
{
    return d_ptr->m_formatter;
}


void tools::log::setGlobalLogLevel(int level)
{
    gLogLevel = level;
}


int tools::log::globalLogLevel()
{
    return gLogLevel;
}


void tools::log::incGlobalLogLevel()
{
    ++gLogLevel;
}
