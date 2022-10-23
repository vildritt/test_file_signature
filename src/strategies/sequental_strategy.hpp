#ifndef SS_STRATEGIES_SEQUENTAL_STRATEGY_H
#define SS_STRATEGIES_SEQUENTAL_STRATEGY_H
#pragma once

#include "strategies/abstract_strategy.hpp"

namespace ss {

/**
 * @brief Simple not-threaded sequental processing strategy.
 * Used as:
 *  - fallback in simple cases
 *  - reference strat. in test of other strategies
 */
class SequentalHashStrategy : public AbstractHashStrategy
{
private:
    void doHash(const Configuration& config) override;
    std::string getConfigurationStringRepresentation() const override;
};

} // ns ss

#endif // SS_STRATEGIES_SEQUENTAL_STRATEGY_H
