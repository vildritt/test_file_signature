#ifndef SS_STRATEGIES_THREADED_H
#define SS_STRATEGIES_THREADED_H
#pragma once

#include "strategies/abstract.hpp"


namespace ss {

/**
 * @brief Threaded processing startegy.
 * lightweight class, main machiner priv. implemented
 */
class ThreadedHashStrategy : public AbstractHashStrategy
{
public:
    /**
     * @param poolSizeHint - hint to use special threads count. 0 - autochoose
     * @param singleThreadSequentalRangeSize - hint for cont. range size in bytes for single thread to process. 0 - autochoose
     */
    ThreadedHashStrategy(size_t poolSizeHint = 0, SizeBytes singleThreadSequentalRangeSize = 0);
    static void setSingleThreadSequentalRangeSize(SizeBytes size);
private:
    size_t m_poolSizeHint = 0;
    SizeBytes m_singleThreadSequentalRangeSize = 0;

    void doHash(const std::string& inFilePath, const ss::DigestWriterPtr& writer, const ss::FileSlicesScheme& slices, const tools::hash::HasherFactoryPtr &hasherFactory) override;
    std::string getConfigurationStringRepresentation() const override;
};

} // ns ss

#endif // SS_STRATEGIES_THREADED_H
