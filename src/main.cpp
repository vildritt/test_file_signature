#include <iostream>
#include <fstream>
#include <filesystem>

#include <cassert>
#include <memory>

#include "consts.hpp"
#include "misc.hpp"
#include "reader.hpp"
#include "writers/stream_writer.hpp"
#include "writers/file_stream_writer.hpp"
#include "strategies/abstract_strategy.hpp"

#include <tools/hash/md5_hasher.hpp>
#include <tools/log.hpp>
#include <tools/timer.hpp>

#include <limits>

TS_LOGGER("main")


void evaluateFileSignature(const misc::Options& opts);
void performanceTest(
        const ss::HashStrategyPtr& strategy,
        const misc::Options& opts,
        const ss::AbstractHashStrategy::Configuration& config);



int main(int argc, const char* argv[])
{
    misc::Options options;
    try {
        options = misc::parseCliParameters(argc, argv);
    } catch (const std::exception& e) {
        misc::printUsage(argv[0]);
        TS_ELOGF("Parsing parameters failed: %s", e.what());
        return 1;
    }

    tools::log::setGlobalLogLevel(options.logLevel);

    try {
        evaluateFileSignature(options);
    } catch (const std::exception& e) {
        TS_ELOG(e.what());
        return 2;
    } catch (...) {
        TS_ELOG("unknown");
        return 2;
    }

    return 0;
}


void evaluateFileSignature(const misc::Options& options)
{
    const bool isNormalModeRun = !options.performanceTest;

    // simple check for input file
    const std::filesystem::path inputFilePath(options.inputFilePath);
    if (!std::filesystem::exists(inputFilePath)) {
        throw std::runtime_error("input file not exists: " + options.inputFilePath);
    }

    ss::AbstractHashStrategy::Configuration config;

    if (isNormalModeRun) {
        if (options.outputFilePath.empty()) {
            config.writer = std::make_shared<ss::StreamDigestWriter>(&std::cout);
        } else {
            config.writer = std::make_shared<ss::FileStreamDigestWriter>(options.outputFilePath);
        }
    }

    config.fileSlicesScheme = ss::FileSlicesScheme(
                std::filesystem::file_size(inputFilePath),
                options.blockSizeBytes,
                options.suggestedReadBufferSize);

    auto strategy = ss::AbstractHashStrategy::chooseStrategy(options.inputFilePath,
                                                     config.fileSlicesScheme,
                                                     options.forcedStrategySymbol);

    assert(strategy.get() != nullptr && "strategy not choosed!");

    config.hasherFactory = std::make_shared<tools::hash::md5::HasherFactory>();
    config.readerfactory = std::make_shared<ss::FileBlockReaderFactoryDelegate>([options, config]() {
        return std::make_shared<ss::FileBlockReader>(
                    options.inputFilePath,
                    config.fileSlicesScheme,
                    config.fileSlicesScheme.suggestedReadBufferSizeBytes);
    });

    if (isNormalModeRun) {
        strategy->hash(config);
    } else {
        performanceTest(strategy, options, config);
    }

    if (config.writer) {
        config.writer->flush();
    }
}


void performanceTest(
        const ss::HashStrategyPtr& strategy,
        const misc::Options& opts,
        const ss::AbstractHashStrategy::Configuration& config)
{
    const std::string name = tools::Formatter()
            .format("perf: FS=[%16lld] BS=[%10d], ST=[%15s]",
                    config.fileSlicesScheme.fileSizeBytes,
                    config.fileSlicesScheme.blockSizeBytes,
                    strategy->configurationStringRepresentation().c_str()).str();

    tools::Timer globalTimer(name, false);

    tools::Timer flushTimer;

    const auto dropCaches = [&globalTimer, &flushTimer]() {
        if (flushTimer.elapsed_s() < ss::kFlushCachesMinPeriod_ms) {
            return;
        }
        flushTimer.start();

        globalTimer.pause();
        misc::dropOSCaches();
        globalTimer.resume();
    };

    size_t done = 0;
    while (done < ss::kPerfTestCountLimit) {
        if (globalTimer.elapsed_s() > ss::kPerfTestMinRunTime_s) {
            break;
        }

        dropCaches();

        strategy->hash(config);
        ++done;
    }

    globalTimer.log(done);
}
