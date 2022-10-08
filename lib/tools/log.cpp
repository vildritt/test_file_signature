#include <tools/log.hpp>

#include <atomic>
#include <iostream>


namespace  {

std::atomic_int gLogLevel(tools::log::Level::Error);

const char* levelToName(int level)
{
    switch(level) {
    case tools::log::Level::Error:
        return "ERRO";
    case tools::log::Level::Warning:
        return "WARN";
    case tools::log::Level::Verbose:
        return "VERB";
    case tools::log::Level::Debug:
        return "DEBG";
    default:
        return "UNKN";
    }
}

}

tools::log::Logger::Logger(const std::string &prefix)
    : m_prefix(prefix)
    , m_level(gLogLevel)
{
}



void tools::log::Logger::log(int level, const char *msg)
{
    if (!checkLevel(level)) {
        return;
    }

    std::lock_guard<std::mutex> guard(m_mutex);

    //TODO 1: add timestamp

    std::cerr << levelToName(level)
              << "["  << m_prefix << "]: "
              << msg
              << std::endl;
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
