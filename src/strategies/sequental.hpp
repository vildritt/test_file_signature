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
class SequentalHashStrategy : public AbstractHashStrategy
{
private:
    void doHash(const std::string& inFilePath, const ss::DigestWriterPtr& writer, const ss::FileSlicesScheme& slices, const tools::hash::HasherFactoryPtr &hasherFactory) override;
    std::string getConfigurationStringRepresentation() const override;
};

} // ns ss

#endif // SS_STRATEGIES_SEQUENTAL_H
