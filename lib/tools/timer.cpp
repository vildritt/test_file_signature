#include <tools/timer.hpp>

#include <tools/log.hpp>


TS_LOGGER("timer");


tools::Timer::Timer(const std::string &name, bool autoLoggable)
    : m_timerName(name)
    , m_autoLoggableOnDestory(autoLoggable)
{
    start();
}


tools::Timer::~Timer() noexcept
{
    try {
        if (m_autoLoggableOnDestory) {
            log();
        }
    }  catch (...) {
    }
}


void tools::Timer::start()
{
    m_startTimePoint = Clock::now();
    m_isStarted = true;
    m_isPaused = false;
    m_elapsedInPausesAccumulator_ns = 0;
}


uint64_t tools::Timer::elapsed_ns(bool restart)
{
    if (!m_isStarted) {
        return 0;
    }

    auto res = Timer::elapsedNsFromTs(m_startTimePoint) - m_elapsedInPausesAccumulator_ns;
    if (m_isPaused) {
        res -= Timer::elapsedNsFromTs(m_pausedAtTimePoint);
    }

    if (restart) {
        this->start();
    }

    return res;
}


double tools::Timer::elapsed_us(bool restart)
{
    return elapsed_ns(restart) * 1.0E-3;
}


double tools::Timer::elapsed_ms(bool restart)
{
    return elapsed_ns(restart) * 1.0E-6;
}


double tools::Timer::elapsed_s(bool restart)
{
    return elapsed_ns(restart) * 1.0E-9;
}


void tools::Timer::pause()
{
    if (m_isPaused || !m_isStarted) {
        return;
    }
    m_pausedAtTimePoint = Clock::now();
    m_isPaused = true;
}


void tools::Timer::resume()
{
    if (!m_isPaused || !m_isStarted) {
        return;
    }
    m_elapsedInPausesAccumulator_ns += Timer::elapsedNsFromTs(m_pausedAtTimePoint);
    m_isPaused = false;
}


void tools::Timer::log(size_t statCount, bool restart)
{
    if (m_isStarted) {
        const auto us = elapsed_us(restart);
        const auto ms = us / 1000.0;
        const auto s = ms / 1000.0;
        if (statCount > 0) {
            TS_NLOGF("[%s] elapsed: T[tot.ms]=%10.3f; STAT: %7d times; T[us]=%13.3f; T[ms]=%10.3f; T[s]=%10.3f)",
                     m_timerName.c_str(),
                     ms,
                     statCount,
                     us / statCount,
                     ms / statCount,
                     s / statCount);
        } else {
            TS_VLOGF("[%s] elapsed: T[tot.ms]=%10.3f", m_timerName.c_str(), ms);
        }
    } else {
        TS_WLOGF("[%s] not started yet", m_timerName.c_str());
    }
    if (m_elapsedInPausesAccumulator_ns > 0) {
        TS_D2LOGF("[%s] elapsed in pause: %10.3f ms", m_timerName.c_str(), m_elapsedInPausesAccumulator_ns * 1.0E-6);
    }
}


uint64_t tools::Timer::elapsedNsFromTs(const Clock::time_point &tp)
{
    return std::chrono::nanoseconds(Clock::now() - tp).count();
}
