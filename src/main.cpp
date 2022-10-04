#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <filesystem>


using Size = uint64_t;

static constexpr const Size kKiloBytes = 1024;
static constexpr const Size kMegaBytes = 1024 * kKiloBytes;

static constexpr const Size kMinBlockSizeBytes = 512;
static constexpr const Size kMaxBlockSizeBytes = 10 * kMegaBytes;

struct Options {
    static constexpr const uint64_t kDefaultBlockSize = 1*1024*1024;
    std::string inputFilePath;
    std::string outputFilePath;
    uint64_t blockSizeBytes = kDefaultBlockSize;
};


void printUsage(const char* appPath)
{
    const auto executableName = std::filesystem::path(appPath).filename();
    std::cerr << "Usage: " << executableName << " <in_file_path> [<out_file_path=>] [<buffer_size=1M>]" << std::endl;
}


uint64_t parseBlockSize(const std::string& blockSizeText)
{
    size_t idx = 0;
    uint64_t res = std::stoull(blockSizeText, &idx, 10);
    if (idx < blockSizeText.size()) {
        switch(blockSizeText[idx]) {
            case 'k':
            case 'K':
                res *= 1024;
                break;
            case 'm':
            case 'M':
                res *= 1024*1024;
                break;
        }
    }
    return res;
}


Options parseCliParameters(int argc, char* argv[])
{
    Options opts;
    if (argc < 2) {
        printUsage(argv[0]);
        throw std::runtime_error("input file path not given");
    }

    opts.inputFilePath = argv[1];

    if (argc >= 3) {
        opts.outputFilePath = argv[2];

        if (argc >= 4) {
            opts.blockSizeBytes = parseBlockSize(argv[3]);
        }
    }

    if (opts.blockSizeBytes < kMinBlockSizeBytes) {
        throw std::runtime_error("block size is less then minimal");
    }
    if (opts.blockSizeBytes < kMaxBlockSizeBytes) {
        throw std::runtime_error("block size is less then maximal");
    }

    return opts;
}


void getSignatures(const Options& opts)
{
    const std::filesystem::path finp(opts.inputFilePath);
    if (!std::filesystem::exists(finp)) {
        throw std::runtime_error("input file not exists: " + opts.inputFilePath);
    }

    const auto size = std::filesystem::file_size(finp);
    const auto blockSize = opts.blockSizeBytes;

    const auto

    std::ifstream fin(opts.inputFilePath, std::ios_base::binary);
    std::ofstream fout(opts.outputFilePath, std::ios_base::binary);

    fin.g
    std::ifstream::pos_type filesize(const char* filename)
    {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }

}

int main(int argc, char* argv[])
{
    try {
        const auto opts = parseCliParameters(argc, argv);

        std::cout
                << opts.inputFilePath << std::endl
                << opts.outputFilePath << std::endl
                << opts.blockSizeBytes << std::endl;

        getSignatures(opts);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what();
        return 1;
    } catch (...) {
        std::cerr << "Error: unknown";
        return 1;
    }
    return 0;
}
