#ifndef SS_STRATEGIES_ABSTRACT_H
#define SS_STRATEGIES_ABSTRACT_H
#pragma once

#include <string>
#include <ostream>
#include <memory>

#include "slices_scheme.hpp"

namespace ss {

class HashStrategy;

using HashStrategyPtr = std::shared_ptr<HashStrategy>;

/**
 * @brief Abstract hasher strategy
 */
class HashStrategy {
public:
    HashStrategy() = default;
    virtual ~HashStrategy() = default;

    HashStrategy(const HashStrategy&) = delete;
    HashStrategy(HashStrategy&&) = delete;
    HashStrategy& operator=(const HashStrategy&) = delete;
    HashStrategy& operator=(HashStrategy&&) = delete;

    /**
     * @brief do get hash digetst of given file
     * @param inFilePath - input file path
     * @param os - optional output stream (can be nullptr for tests)
     * @param slices - split file to blocks strategy
     */
    void hash(const std::string& inFilePath, std::ostream *os, const SlicesScheme &slices);

    /**
     * @brief short descriptive string of strategy configuration. Used in debug logging
     */
    std::string confString() const;
private:
    virtual void doHash(const std::string& inFilePath, std::ostream* os, const ss::SlicesScheme& slices) = 0;
    virtual std::string getConfString() const;
public:

    /**
     * @brief default strategy chooser
     */
    static ss::HashStrategyPtr chooseStrategy(const std::string& filePath,
            SlicesScheme &slices,
            const std::string& forcedStrategySymobl);
};


} // ns ss

#endif // SS_STRATEGIES_ABSTRACT_H
