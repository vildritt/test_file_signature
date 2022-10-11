#include <iostream>
#include <fstream>
#include <filesystem>

#include <cassert>
#include <memory>

#include "misc.hpp"
#include "strategies/abstract.hpp"
#include "consts.hpp"
#include "writers/stream_writer.hpp"
#include "writers/file_stream_writer.hpp"

#include <tools/hash/md5_hasher.hpp>
#include <tools/log.hpp>
#include <tools/timer.hpp>

#include <limits>

TS_LOGGER("main")


void evaluateFileSignature(const misc::Options& opts);
void performanceTest(
        const ss::HashStrategyPtr& strategy,
        const misc::Options& opts,
        const ss::FileSlicesScheme& slices,
        const tools::hash::HasherFactoryPtr &hasherFactory);



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

    ss::DigestWriterPtr writer;

    if (isNormalModeRun) {
        if (options.outputFilePath.empty()) {
            writer = std::make_shared<ss::StreamDigestWriter>(&std::cout);
        } else {
            writer = std::make_shared<ss::FileStreamDigestWriter>(options.outputFilePath);
        }
    }

    ss::FileSlicesScheme slices(
                std::filesystem::file_size(inputFilePath),
                options.blockSizeBytes,
                options.suggestedReadBufferSize);

    auto strategy = ss::AbstractHashStrategy::chooseStrategy(options.inputFilePath,
                                                     slices,
                                                     options.forcedStrategySymbol);

    assert(strategy.get() != nullptr && "strategy not choosed!");

    const auto hasherFactory = std::make_shared<tools::hash::md5::HasherFactory>();

    if (isNormalModeRun) {
        strategy->hash(options.inputFilePath, writer, slices, hasherFactory);
    } else {
        performanceTest(strategy, options, slices, hasherFactory);
    }

    if (writer) {
        writer->flush();
    }
}


void performanceTest(
        const ss::HashStrategyPtr& strategy,
        const misc::Options& opts,
        const ss::FileSlicesScheme& slices,
        const tools::hash::HasherFactoryPtr& hasherFactory)
{
    const std::string name = tools::Formatter()
            .format("perf: FS=[%16lld] BS=[%10d], ST=[%15s]",
                    slices.fileSizeBytes,
                    slices.blockSizeBytes,
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

        strategy->hash(opts.inputFilePath, nullptr, slices, hasherFactory);
        ++done;
    }

    globalTimer.log(done);
}
