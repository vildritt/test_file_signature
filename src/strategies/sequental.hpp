#ifndef SS_STRATEGIES_SEQUENTAL_H
#define SS_STRATEGIES_SEQUENTAL_H
#pragma once

#include "strategies/abstract.hpp"

namespace ss {

class SequentalHashStrategy : public HashStrategy
{
private:
    void doHash(const std::string& inFilePath, std::ostream& os, const ss::SlicesScheme& slices) override;
};

} // ns ss

#endif // SS_STRATEGIES_SEQUENTAL_H
