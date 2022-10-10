#ifndef SS_STRATEGIES_THREADED_H
#define SS_STRATEGIES_THREADED_H
#pragma once

#include "strategies/abstract.hpp"


namespace ss {

class ThreadedHashStrategy : public HashStrategy
{
public:
    ThreadedHashStrategy(size_t blocksPerThread = 0, size_t poolSizeHint = 0, SizeBytes singleThreadSequentalRangeSize = 0);
    static void setSingleThreadSequentalRangeSize(SizeBytes size);
private:
    size_t m_poolSizeHint = 0;
    size_t m_blocksPerThread = 0;
    SizeBytes m_singleThreadSequentalRangeSize = 0;

    void doHash(const std::string& inFilePath, std::ostream* os, const ss::SlicesScheme& slices) override;
    std::string getConfString() const override;

};

} // ns ss

#endif // SS_STRATEGIES_THREADED_H
