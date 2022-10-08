#include <iostream>
#include <fstream>
#include <filesystem>

#include <cassert>
#include <memory>

#include "misc.hpp"
#include "strategies/abstract.hpp"

#include <tools/log.hpp>

TS_LOGGER("main")

void evalSignature(const misc::Options& opts)
{
    // simple check for input file
    const std::filesystem::path finp(opts.inputFilePath);
    if (!std::filesystem::exists(finp)) {
        throw std::runtime_error("input file not exists: " + opts.inputFilePath);
    }

    // choose output stream
    std::ostream* sout = &std::cout;
    std::unique_ptr<std::ofstream> fout;
    if (!opts.outputFilePath.empty()) {
        fout = std::make_unique<std::ofstream>(opts.outputFilePath, std::ios_base::trunc);
        sout = fout.get();

        if (!fout->is_open()) {
            throw std::runtime_error("failed to open output file: " + opts.inputFilePath);
        }
    }

    const ss::SlicesScheme slices(std::filesystem::file_size(finp), opts.blockSizeBytes);
    auto strategy = ss::HashStrategy::chooseStrategy(opts.inputFilePath,
                                                     slices,
                                                     opts.forcedStrategySymbol);

    assert(strategy.get() != nullptr && "strategy not choosed");
    strategy->hash(opts.inputFilePath, *sout, slices);

    *sout << std::flush;
}


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
