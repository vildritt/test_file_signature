#include "threaded.hpp"

#include <thread>

#include "strategies/threaded/shared_context.hpp"
#include "consts.hpp"

#include <tools/formatter.hpp>


ss::ThreadedHashStrategy::ThreadedHashStrategy(size_t poolSizeHint, SizeBytes singleThreadSequentalRangeSize)
    : m_poolSizeHint(poolSizeHint)
    , m_singleThreadSequentalRangeSize(singleThreadSequentalRangeSize)
{
    if (m_poolSizeHint == 0) {
        m_poolSizeHint = std::thread::hardware_concurrency();
    }
}


void ss::ThreadedHashStrategy::doHash(const std::string &inFilePath, const ss::DigestWriterPtr& writer, const FileSlicesScheme &slices, const tools::hash::HasherFactoryPtr &hasherFactory)
{
    SizeBytes effSingleThreadSequentalRangeSize = m_singleThreadSequentalRangeSize > 0
            ? m_singleThreadSequentalRangeSize
            : slices.suggestedReadBufferSizeBytes;

    if (effSingleThreadSequentalRangeSize == 0) {
        effSingleThreadSequentalRangeSize = ss::kDefaultSingleThreadSequentalRangeSize;
    }

    ss::threaded::SharedContext ctx(inFilePath, slices, m_poolSizeHint, effSingleThreadSequentalRangeSize, hasherFactory);

    ctx.run(writer);
}


std::string ss::ThreadedHashStrategy::getConfigurationStringRepresentation() const
{
    return tools::Formatter().format("T:%d:%lld",
                                     m_poolSizeHint,
                                     m_singleThreadSequentalRangeSize).str();
}
