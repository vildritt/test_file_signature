#ifndef SS_MISC_H
#define SS_MISC_H
#pragma once

#include <string>

#include "types.hpp"
#include "consts.hpp"


namespace misc {

/**
 * @brief do print usage text
 * @param appPath - optional application executable path
 */
void printUsage(const char* appPath);

/**
 * @brief do parse block size from string with suffixes support like: 1K, 20M
 */
ss::SizeBytes parseBlockSize(const std::string &blockSizeText);


/**
 * @brief Application options from cli
 */
struct Options {
    static constexpr const ss::SizeBytes kDefaultBlockSize = ss::kDefaultBlockSize;

    std::string inputFilePath;
    std::string outputFilePath;
    ss::SizeBytes blockSizeBytes = kDefaultBlockSize;
    ss::SizeBytes suggestedReadBufferSize = 0;
    /**
     * @brief used for debug diff strategies
     */
    std::string forcedStrategySymbol;
    /**
     * @brief do run perf tests @see main.cpp
     */
    bool performanceTest = false;
    int logLevel = 0;
};


/**
 * @brief parse options from cli
 */
Options parseCliParameters(int argc, const char* argv[]);

/**
 * @brief try to guess file media from file path. NOT IMPLEMENTED,
 * @return default value
 */
ss::MediaType guessFileMediaType(const std::string& path);

/**
 * @brief try to suggest buffer size depending on media type
 * @param blockSize - hashable block size in bytes
 */
ss::SizeBytes suggestReadBufferSizeByMediaType(ss::MediaType mt, ss::SizeBytes blockSize);

/**
 * @brief do drop OS caches. Used in performance tests
 */
void dropOSCaches();

} // ns misc

#endif // SS_MISC_H
