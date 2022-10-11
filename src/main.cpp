#include <iostream>
#include <fstream>
#include <filesystem>

#include <cassert>
#include <memory>

#include "misc.hpp"
#include "strategies/abstract.hpp"
#include "consts.hpp"

#include <tools/log.hpp>
#include <tools/timer.hpp>

TS_LOGGER("main")


void evalSignature(const misc::Options& opts);
void performanceTest(const ss::HashStrategyPtr& strategy, const misc::Options& opts, const ss::SlicesScheme& slices);


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
        evalSignature(options);
    } catch (const std::exception& e) {
        TS_ELOG(e.what());
        return 2;
    } catch (...) {
        TS_ELOG("unknown");
        return 2;
    }

    return 0;
}


void evalSignature(const misc::Options& opts)
{
    // simple check for input file
    const std::filesystem::path finp(opts.inputFilePath);
    if (!std::filesystem::exists(finp)) {
        throw std::runtime_error("input file not exists: " + opts.inputFilePath);
    }

    // choose output stream
    std::ostream* sout = opts.performanceTest
            ? nullptr
            : &std::cout;

    std::unique_ptr<std::ofstream> fout;
    if (!opts.outputFilePath.empty()) {
        fout = std::make_unique<std::ofstream>(opts.outputFilePath, std::ios_base::trunc);
        sout = fout.get();

        if (!fout->is_open()) {
            throw std::runtime_error("failed to open output file: " + opts.inputFilePath);
        }
    }

    ss::SlicesScheme slices(std::filesystem::file_size(finp), opts.blockSizeBytes);
    slices.suggestedReadBufferSize = opts.suggestedReadBufferSize;
    auto strategy = ss::HashStrategy::chooseStrategy(opts.inputFilePath,
                                                     slices,
                                                     opts.forcedStrategySymbol);

    assert(strategy.get() != nullptr && "strategy not choosed");

    if (!opts.performanceTest) {
        strategy->hash(opts.inputFilePath, sout, slices);
    } else {
        performanceTest(strategy, opts, slices);
    }

    if (sout) {
        *sout << std::flush;
    }
}


void performanceTest(const ss::HashStrategyPtr& strategy, const misc::Options& opts, const ss::SlicesScheme& slices)
{
    const std::string name = tools::Formatter()
            .format("perf: FS=[%16lld] BS=[%10d], ST=[%15s]",
                    slices.dataSize,
                    slices.blockSize,
                    strategy->confString().c_str()).str();

    tools::Timer globalTimer(name, false);

    tools::Timer flushTimer;

    const auto dropCaches = [&globalTimer, &flushTimer]() {
        if (flushTimer.elapsed_s() < ss::kFlushCachesMinPeriod_ms) {
            return;
        }
        flushTimer.start();

        globalTimer.pause();
        misc::dropOSCaches();
        globalTimer.cont();
    };

    size_t done = 0;
    while (done < ss::kPerfTestCountLimit) {
        if (globalTimer.elapsed_s() > ss::kPerfTestMinRunTime_s) {
            break;
        }

        dropCaches();

        strategy->hash(opts.inputFilePath, nullptr, slices);
        ++done;
    }

    globalTimer.log(done);
}
