#ifndef SS_STRATEGIES_SEQUENTAL_H
#define SS_STRATEGIES_SEQUENTAL_H
#pragma once

#include "strategies/abstract.hpp"

namespace ss {

/**
 * @brief Simple one threaded sequental processing strategy.
 * Used as
 *  - fallback in simple cases
 *  - reference start. in test of other strategies
 */
class SequentalHashStrategy : public HashStrategy
{
private:
    void doHash(const std::string& inFilePath, std::ostream* os, const ss::SlicesScheme& slices) override;
    std::string getConfString() const override;
};

} // ns ss

#endif // SS_STRATEGIES_SEQUENTAL_H
