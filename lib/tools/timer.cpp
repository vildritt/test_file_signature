#include <tools/timer.hpp>

#include <tools/log.hpp>


TS_LOGGER("timer");


tools::Timer::Timer(const std::string &name, bool autoLoggable)
    : m_name(name)
    , m_autoLoggable(autoLoggable)
{
    start();
}


tools::Timer::~Timer() noexcept
{
    try {
        if (m_autoLoggable) {
            log();
        }
    }  catch (...) {
    }
}


void tools::Timer::start()
{
    m_start_tp = Clock::now();
    m_started = true;
    m_paused = false;
    m_elapsedInPausesAccum_ns = 0;
}


uint64_t tools::Timer::elapsed_ns(bool restart)
{
    if (!m_started) {
        return 0;
    }

    auto res = elapsedNsFromTs(m_start_tp) - m_elapsedInPausesAccum_ns;
    if (m_paused) {
        res -= elapsedNsFromTs(m_pause_tp);
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
    if (m_paused || !m_started) {
        return;
    }
    m_pause_tp = Clock::now();
    m_paused = true;
}


void tools::Timer::cont()
{
    if (!m_paused || !m_started) {
        return;
    }
    m_elapsedInPausesAccum_ns += elapsedNsFromTs(m_pause_tp);
    m_paused = false;
}


void tools::Timer::log(int statCount, bool restart)
{
    if (m_started) {
        const auto us = elapsed_us(restart);
        const auto ms = us / 1000.0;
        const auto s = ms / 1000.0;
        if (statCount > 0) {
            TS_NLOGF("[%s] elapsed: T[tot.ms]=%10.3f; STAT: %7d times; T[us]=%13.3f; T[ms]=%10.3f; T[s]=%10.3f)",
                     m_name.c_str(),
                     ms,
                     statCount,
                     us / statCount,
                     ms / statCount,
                     s / statCount);
        } else {
            TS_VLOGF("[%s] elapsed: T[tot.ms]=%10.3f", m_name.c_str(), ms);
        }
    } else {
        TS_WLOGF("[%s] not started yet", m_name.c_str());
    }
    if (m_elapsedInPausesAccum_ns > 0) {
        TS_D2LOGF("[%s] elapsed in pause: %10.3f ms", m_name.c_str(), m_elapsedInPausesAccum_ns * 1.0E-6);
    }
}


uint64_t tools::Timer::elapsedNsFromTs(const Clock::time_point& tp) const
{
    return std::chrono::nanoseconds(Clock::now() - tp).count();
}
