#ifndef LIB_TOOLS_TIMER_H
#define LIB_TOOLS_TIMER_H
#pragma once

#include <string>
#include <chrono>
#include <cstdint>

namespace tools {

/**
 * @brief High-resolution timer to measure and log periods
 * MT: not thread-safe
 */
class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    /**
     * on ctor start also called
     * @param name - will be used in logging
     * @param autoLoggable - do log on dtor
     */
    Timer(const std::string& name = std::string(), bool autoLoggable = false);
    ~Timer() noexcept;

    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    /**
     * @brief restart timer
     */
    void start();

    /**
     * @brief pause time counting, to resume - use @resume
     * if already paused or not started - do nothing
     */
    void pause();

    /**
     * @brief resume after pause.
     * if not paused or not started - do nothing
     */
    void resume();

    /**
     * getters of elapsed time from start exclude paused periods
     * @param restart - do restart timer after read value
     */
    uint64_t elapsed_ns(bool restart = false);
    double elapsed_us(bool restart = false);
    double elapsed_ms(bool restart = false);
    double elapsed_s(bool restart = false);

    /**
     * @brief do log in def timer logger
     * @param statCount - if >0 - do div elapsed time by this and show elapsed time per item stat
     * @param restart - see @elapsed_*
     * SIDE EFFECT: log to default timer logger
     */
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
