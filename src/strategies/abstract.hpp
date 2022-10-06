#ifndef SS_STRATEGIES_ABSTRACT_H
#define SS_STRATEGIES_ABSTRACT_H
#pragma once

#include <string>
#include <ostream>
#include <memory>

#include "slices_scheme.hpp"

namespace ss {

class HashStrategy {
public:
    HashStrategy() = default;
    virtual ~HashStrategy() = default;

    HashStrategy(const HashStrategy&) = delete;
    HashStrategy(HashStrategy&&) = delete;
    HashStrategy& operator=(const HashStrategy&) = delete;
    HashStrategy& operator=(HashStrategy&&) = delete;

    void hash(const std::string& inFilePath, std::ostream& os, const ss::SlicesScheme& slices);
private:
    virtual void doHash(const std::string& inFilePath, std::ostream& os, const ss::SlicesScheme& slices) = 0;

public:

    static std::unique_ptr<HashStrategy> chooseStrategy(
            const std::string& filePath,
            const ss::SlicesScheme& slices,
            const std::string& forcedStrategySymobl);
};

} // ns ss

#endif // SS_STRATEGIES_ABSTRACT_H
