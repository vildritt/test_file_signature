#ifndef LIB_TOOLS_TIMER_H
#define LIB_TOOLS_TIMER_H
#pragma once

#include <string>
#include <chrono>
#include <cstdint>

namespace tools {

class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    Timer(const std::string& name = std::string(), bool autoLoggable = false);
    ~Timer() noexcept;

    void start();
    void pause();
    void cont();

    uint64_t elapsed_ns(bool restart = false);
    double elapsed_us(bool restart = false);
    double elapsed_ms(bool restart = false);
    double elapsed_s(bool restart = false);

    void log(size_t statCount = 0, bool restart = false);

private:
    std::string m_name;
    bool m_autoLoggable = false;

    Clock::time_point m_start_tp;
    Clock::time_point m_pause_tp;
    bool m_started = false;
    bool m_paused = false;
    uint64_t m_elapsedInPausesAccum_ns = 0;

    static uint64_t elapsedNsFromTs(const Clock::time_point &tp);
};


} // ns tools

#endif // LIB_TOOLS_TIMER_H
