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
    Options opts;
    if (argc < 2) {
        throw std::runtime_error("input file path not given");
    }

    int posArgIndex = -1;
    for(int i = 1; i < argc; ++i) {
        const std::string_view a(argv[i]);
        if (a.empty()) {
            continue;
        }
        const size_t L = a.size();

        // check - is it flag?
        if (a[0] == '-' && L > 1) {
            for(size_t j = 1; j < L; ++j) {
                switch(a[j]) {
                    case 'd':
                        ++opts.logLevel;
                        break;
                    case 'p':
                        opts.performanceTest = true;
                        break;
                }
            }
            continue;
        }

        // it's positional
        ++posArgIndex;

        switch(posArgIndex) {
        case 0:
            opts.inputFilePath = a;
            break;
        case 1:
            opts.outputFilePath = a;
            break;
        case 2:
            opts.blockSizeBytes = misc::parseBlockSize(std::string(a));
            break;
        case 3:
            opts.forcedStrategySymbol = a;
            break;
        case 4:
            opts.suggestedReadBufferSize = misc::parseBlockSize(std::string(a));
            break;
        }
    }

    // fixes
    if (opts.outputFilePath == "-") {
        opts.outputFilePath = "";
    }
    if (opts.blockSizeBytes <= 0) {
        opts.blockSizeBytes = Options::kDefaultBlockSize;
    }

    // checks
    if (opts.inputFilePath.empty()) {
        throw std::runtime_error("input file must be given");
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


ss::SizeBytes misc::suggestReadBufferSizeByMediaType(ss::MediaType mt, ss::SizeBytes blockSize)
{
    //TODO 0: tune values
    switch (mt) {
    case ss::MediaType::Memory:
        return std::max(8 * ss::kMegaBytes, blockSize);
    case ss::MediaType::SSD:
        return std::max(2 * ss::kMegaBytes, blockSize);
    case ss::MediaType::HDD:
         // tests (1 laptop) shows for small blocks after 1 MB no perf grow
        return std::max(1 * ss::kMegaBytes, blockSize);
    case ss::MediaType::Unknown:
        return 1 * ss::kMegaBytes;
    case ss::MediaType::NetworkDrive:
        // no tests, assume ill connected
        return 512 * ss::kKiloBytes;
    }
    return 1 * ss::kMegaBytes;
}

