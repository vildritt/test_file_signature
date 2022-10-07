#include <iostream>
#include <fstream>
#include <filesystem>

#include <cassert>
#include <memory>

#include "misc.hpp"
#include "strategies/abstract.hpp"


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
    try {
        const auto options = misc::parseCliParameters(argc, argv);
        evalSignature(options);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: unknown" << std::endl;
        return 1;
    }
    return 0;
}
