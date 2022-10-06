#ifndef SS_MISC_H
#define SS_MISC_H
#pragma once

#include <string>

#include "types.hpp"
#include "consts.hpp"


namespace misc {

void printUsage(const char* appPath);
ss::SizeBytes parseBlockSize(const std::string& blockSizeText);


struct Options {
    static constexpr const ss::SizeBytes kDefaultBlockSize = ss::kDefaultBlockSize;

    std::string inputFilePath;
    std::string outputFilePath;
    ss::SizeBytes blockSizeBytes = kDefaultBlockSize;
    std::string forcedStrategySymbol;
};


Options parseCliParameters(int argc, const char* argv[]);

ss::MediaType guessFileMediaType(const std::string& path);

} // ns misc

#endif // SS_MISC_H
