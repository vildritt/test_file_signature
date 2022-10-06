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
};


Options parseCliParameters(int argc, const char* argv[]);


} // ns misc

#endif // SS_MISC_H
