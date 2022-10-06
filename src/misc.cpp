#include "misc.hpp"

#include <filesystem>
#include <iostream>


namespace  {

const char * kUsage =
#include "usage.txt"
;

} // ns a

void misc::printUsage(const char *appPath)
{
    const std::string kPlaceHolder = "%TOOL_NAME%";

    std::string usage = kUsage;

    const auto p = usage.find(kPlaceHolder);
    if (p != std::string::npos) {
        const auto executableName = std::filesystem::path(appPath).filename();
        usage = usage.replace(p, kPlaceHolder.size(), executableName);
    }

    std::cerr << usage << std::endl << std::endl;
}


ss::SizeBytes misc::parseBlockSize(const std::string &blockSizeText)
{
    size_t idx = 0;
    ss::SizeBytes res = std::stoull(blockSizeText, &idx, 10);
    if (idx < blockSizeText.size()) {
        switch(blockSizeText[idx]) {
        case 'k':
        case 'K':
            res *= ss::kKiloBytes;
            break;
        case 'm':
        case 'M':
            res *= ss::kMegaBytes;
            break;
        }
    }
    return res;
}


misc::Options misc::parseCliParameters(int argc, const char *argv[])
{
    Options opts;
    if (argc < 2) {
        misc::printUsage(argv[0]);
        throw std::runtime_error("input file path not given");
    }

    opts.inputFilePath = argv[1];

    if (argc >= 3) {
        opts.outputFilePath = argv[2];
        if (opts.outputFilePath == "-") {
            opts.outputFilePath = "";
        }

        if (argc >= 4) {
            opts.blockSizeBytes = misc::parseBlockSize(argv[3]);
            if (argc >= 5) {
                opts.forcedStrategySymbol = argv[4];
            }
        }
    }

    if (opts.blockSizeBytes < ss::kMinBlockSizeBytes) {
        throw std::runtime_error("block size is less then minimal");
    }
    if (opts.blockSizeBytes > ss::kMaxBlockSizeBytes) {
        throw std::runtime_error("block size is greater then maximal");
    }

    return opts;
}


ss::MediaType misc::guessFileMediaType(const std::string &path)
{
    //TODO 0!!!: implement
    return ss::MediaType::Unknown;
}
