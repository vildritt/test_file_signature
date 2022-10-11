#include "misc.hpp"

#include <filesystem>
#include <iostream>
#ifndef _WIN32
#include <sys/wait.h>
#endif

#include <tools/log.hpp>


TS_LOGGER("misc")


namespace  {

const char * kUsage =
#include "usage.txt"
;

} // ns a


void misc::printUsage(const char *appPath)
{
    const std::string kPlaceHolder = "%TOOL_NAME%";

    std::string usage = kUsage;

    if (appPath != nullptr) {
        const auto p = usage.find(kPlaceHolder);
        if (p != std::string::npos) {
            const auto executableName = std::filesystem::path(appPath).filename();
            usage = usage.replace(p, kPlaceHolder.size(), executableName.string());
        }
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
        case 'g':
        case 'G':
            res *= ss::kGigaBytes;
            break;
        }
    }
    return res;
}


misc::Options misc::parseCliParameters(int argc, const char *argv[])
{
    Options options;
    if (argc < 2) {
        throw std::runtime_error("input file path not given");
    }

    int positionalArgumentIndex = -1;
    for(int i = 1; i < argc; ++i) {
        const std::string_view currArg(argv[i]);
        if (currArg.empty()) {
            continue;
        }
        const size_t argLength = currArg.size();

        // check - is it flag?
        if (currArg[0] == '-' && argLength > 1) {
            for(size_t j = 1; j < argLength; ++j) {
                switch(currArg[j]) {
                    case 'd':
                        ++options.logLevel;
                        break;
                    case 'p':
                        options.performanceTest = true;
                        break;
                }
            }
            continue;
        }

        // it's positional
        ++positionalArgumentIndex;

        switch(positionalArgumentIndex) {
        case 0:
            options.inputFilePath = currArg;
            break;
        case 1:
            options.outputFilePath = currArg;
            break;
        case 2:
            options.blockSizeBytes = misc::parseBlockSize(std::string(currArg));
            break;
        case 3:
            options.forcedStrategySymbol = currArg;
            break;
        case 4:
            options.suggestedReadBufferSize = misc::parseBlockSize(std::string(currArg));
            break;
        }
    }

    // fixes
    if (options.outputFilePath == "-") {
        options.outputFilePath = "";
    }
    if (options.blockSizeBytes <= 0) {
        options.blockSizeBytes = Options::kDefaultBlockSize;
    }

    // checks
    if (options.inputFilePath.empty()) {
        throw std::runtime_error("input file must be given");
    }
    if (options.blockSizeBytes < ss::kMinBlockSizeBytes) {
        throw std::runtime_error("block size is less then minimal");
    }
    if (options.blockSizeBytes > ss::kMaxBlockSizeBytes) {
        throw std::runtime_error("block size is greater then maximal");
    }

    return options;
}


ss::MediaType misc::guessFileMediaType(const std::string &filePath)
{
    //TODO 0: implement!
    //    cat /sys/block/sda/queue/rotational
    //    https://stackoverflow.com/questions/2337139/where-is-a-file-mounted
    //    win/lin diff procs

    return ss::MediaType::Unknown;
}


void misc::dropOSCaches()
{
#ifdef _WIN32
    //TODO 1: implement
    TS_WLOG("dropOSCaches not implemented");
#else
    std::system("sync");
    const auto res = std::system("echo 3 > /proc/sys/vm/drop_caches");
    if (WEXITSTATUS(res) != 0) { //TODO 1: impr priv check
        TS_WLOG("root priveleges requered to drop OS caches");
    }
#endif
}


ss::SizeBytes misc::suggestReadBufferSizeByMediaType(ss::MediaType mediaType, ss::SizeBytes blockSizeBytes)
{
    //TODO 0: tune suggested values after perf tests
    switch (mediaType) {
    case ss::MediaType::Memory:
        return std::max(8 * ss::kMegaBytes, blockSizeBytes);
    case ss::MediaType::SSD:
        return std::max(2 * ss::kMegaBytes, blockSizeBytes);
    case ss::MediaType::HDD:
         // tests (1 laptop) shows for small blocks after 1 MB no perf grow
        return std::max(1 * ss::kMegaBytes, blockSizeBytes);
    case ss::MediaType::Unknown:
        return 1 * ss::kMegaBytes;
    case ss::MediaType::NetworkDrive:
        // no tests, assume ill connected
        return 512 * ss::kKiloBytes;
    }
    return 1 * ss::kMegaBytes;
}

