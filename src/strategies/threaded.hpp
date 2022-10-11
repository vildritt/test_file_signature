#ifndef SS_STRATEGIES_THREADED_H
#define SS_STRATEGIES_THREADED_H
#pragma once

#include "strategies/abstract.hpp"


namespace ss {

/**
 * @brief Threaded processing startegy
 */
class ThreadedHashStrategy : public HashStrategy
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

    void doHash(const std::string& inFilePath, std::ostream* os, const ss::SlicesScheme& slices) override;
    std::string getConfString() const override;

};

} // ns ss

#endif // SS_STRATEGIES_THREADED_H
