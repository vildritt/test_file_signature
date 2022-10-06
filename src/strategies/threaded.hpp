#ifndef SS_STRATEGIES_THREADED_H
#define SS_STRATEGIES_THREADED_H
#pragma once

#include "strategies/abstract.hpp"


namespace ss {

class ThreadedHashStrategy : public HashStrategy
{
public:
    ThreadedHashStrategy(size_t poolSizeHint = 0);
private:
    size_t m_poolSizeHint = 0;
    void doHash(const std::string& inFilePath, std::ostream& os, const ss::SlicesScheme& slices) override;
};

} // ns ss

#endif // SS_STRATEGIES_THREADED_H
