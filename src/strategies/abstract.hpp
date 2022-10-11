#ifndef SS_STRATEGIES_ABSTRACT_H
#define SS_STRATEGIES_ABSTRACT_H
#pragma once

#include <string>
#include <ostream>
#include <memory>

#include <tools/hash/hasher.hpp>

#include "slices_scheme.hpp"
#include "writers/writer.hpp"

namespace ss {


class AbstractHashStrategy;


using HashStrategyPtr = std::shared_ptr<AbstractHashStrategy>;


/**
 * @brief Abstract hasher strategy
 */
class AbstractHashStrategy {
public:
    AbstractHashStrategy() = default;
    virtual ~AbstractHashStrategy() = default;

    AbstractHashStrategy(const AbstractHashStrategy&) = delete;
    AbstractHashStrategy(AbstractHashStrategy&&) = delete;
    AbstractHashStrategy& operator=(const AbstractHashStrategy&) = delete;
    AbstractHashStrategy& operator=(AbstractHashStrategy&&) = delete;

    /**
     * @brief do get hash digest of given file
     * @param inFilePath - input file path
     * @param writer - [optional] results writer (can be nullptr for tests)
     * @param slices - split file to blocks scheme
     * @param hasherFactory - concrete hasher producer
     */

    void hash(const std::string& inFilePath, const ss::DigestWriterPtr& writer, const FileSlicesScheme &slices, const tools::hash::HasherFactoryPtr& hasherFactory);

    /**
     * @brief short descriptive string of strategy configuration. Used in debug logging
     */
    std::string configurationStringRepresentation() const;
private:
    virtual void doHash(const std::string& inFilePath, const ss::DigestWriterPtr& writer, const ss::FileSlicesScheme& slices, const tools::hash::HasherFactoryPtr &hasherFactory) = 0;
    virtual std::string getConfigurationStringRepresentation() const;
public:

    /**
     * @brief default strategy chooser
     */
    static ss::HashStrategyPtr chooseStrategy(const std::string& filePath,
            FileSlicesScheme &slices,
            const std::string& forcedStrategySymobl);
};


} // ns ss

#endif // SS_STRATEGIES_ABSTRACT_H
