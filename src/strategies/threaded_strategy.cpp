#include "threaded_strategy.hpp"

#include <thread>

#include <tools/formatter.hpp>

#include "consts.hpp"
#include "strategies/detail/threaded/processor.hpp"


ss::ThreadedHashStrategy::ThreadedHashStrategy(size_t poolSizeHint, SizeBytes singleThreadSequentalRangeSize)
    : m_poolSizeHint(poolSizeHint)
    , m_singleThreadSequentalRangeSize(singleThreadSequentalRangeSize)
{
    if (m_poolSizeHint == 0) {
        m_poolSizeHint = std::thread::hardware_concurrency();
    }
}


void ss::ThreadedHashStrategy::doHash(const Configuration &config)
{
    SizeBytes effSingleThreadSequentalRangeSize = m_singleThreadSequentalRangeSize > 0
            ? m_singleThreadSequentalRangeSize
            : config.fileSlicesScheme.suggestedReadBufferSizeBytes;

    if (effSingleThreadSequentalRangeSize == 0) {
        effSingleThreadSequentalRangeSize = ss::kDefaultSingleThreadSequentalRangeSize;
    }

    ss::detail::threaded::ThreadedHashProcessor ctx(config, m_poolSizeHint, effSingleThreadSequentalRangeSize);

    ctx.run(config.writer);
}


std::string ss::ThreadedHashStrategy::getConfigurationStringRepresentation() const
{
    return tools::Formatter().format("T:%d:%lld",
                                     m_poolSizeHint,
                                     m_singleThreadSequentalRangeSize).str();
}
