#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <vector>


using Size = uint64_t;
using Digest = std::vector<unsigned char>;

static constexpr const Size kKiloBytes = 1024;
static constexpr const Size kMegaBytes = 1024 * kKiloBytes;

static constexpr const Size kMinBlockSizeBytes = 512;
static constexpr const Size kMaxBlockSizeBytes = 10 * kMegaBytes;


struct Options {
    static constexpr const Size kDefaultBlockSize = 1 * kMegaBytes;
    std::string inputFilePath;
    std::string outputFilePath;
    Size blockSizeBytes = kDefaultBlockSize;
};


void printUsage(const char* appPath)
{
    const auto executableName = std::filesystem::path(appPath).filename();
    std::cerr << "Usage: " << executableName << " <in_file_path> [<out_file_path=>] [<buffer_size=1M>]" << std::endl;
}


Size parseBlockSize(const std::string& blockSizeText)
{
    size_t idx = 0;
    Size res = std::stoull(blockSizeText, &idx, 10);
    if (idx < blockSizeText.size()) {
        switch(blockSizeText[idx]) {
            case 'k':
            case 'K':
                res *= kKiloBytes;
                break;
            case 'm':
            case 'M':
                res *= kMegaBytes;
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
        if (opts.outputFilePath == "-") {
            opts.outputFilePath = "";
        }

        if (argc >= 4) {
            opts.blockSizeBytes = parseBlockSize(argv[3]);
        }
    }

    if (opts.blockSizeBytes < kMinBlockSizeBytes) {
        throw std::runtime_error("block size is less then minimal");
    }
    if (opts.blockSizeBytes > kMaxBlockSizeBytes) {
        throw std::runtime_error("block size is greater then maximal");
    }

    return opts;
}


Digest blockDigest(const char* data, Size size)
{
    // TODO 0: implement MD5

    Size v = 0;
    for(Size i = 0; i < size; ++i) {
        v += data[i];
    }

    Digest res;
    const size_t N = sizeof(Size);
    res.reserve(N);
    for(size_t i = 0; i < N; ++i) {
        res.push_back(v & 0xFF);
        v >>= 8;
    }
    return res;
}


void getSignatures(const Options& opts)
{
    assert(opts.blockSizeBytes > 0);

    const std::filesystem::path finp(opts.inputFilePath);
    if (!std::filesystem::exists(finp)) {
        throw std::runtime_error("input file not exists: " + opts.inputFilePath);
    }

    const auto size = std::filesystem::file_size(finp);
    const auto blockSize = opts.blockSizeBytes;
    auto blockCount = size / blockSize;
    const auto lastBlockRealSize = size - blockCount * blockSize;
    const bool needToFillLastBlock = lastBlockRealSize > 0;
    if (needToFillLastBlock) {
        blockCount++;
    }

    std::ifstream fin(opts.inputFilePath, std::ios_base::binary);

    std::ostream* sout = &std::cout;
    std::unique_ptr<std::ofstream> fout;

    if (!opts.outputFilePath.empty()) {
        fout = std::make_unique<std::ofstream>(opts.outputFilePath, std::ios_base::trunc);
        sout = fout.get();
    }

    std::vector<char> block(blockSize);

    const auto lastBlockIndex = blockCount - 1;
    for(decltype(blockCount) i = 0; i < blockCount; ++i) {
        const bool islast = (i == lastBlockIndex);
        if (islast && needToFillLastBlock) {
            fin.read(block.data(), lastBlockRealSize);
            std::fill(block.begin() + lastBlockRealSize, block.end(), 0);
        } else {
            fin.read(block.data(), blockSize);
        }

        // output
        {
            const auto digest = blockDigest(block.data(), block.size());
            for (const auto& v : digest) {
                *sout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(v);
            }
            *sout << std::endl;
        }
    }
}


int main(int argc, char* argv[])
{
    try {
        const auto opts = parseCliParameters(argc, argv);
        getSignatures(opts);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: unknown" << std::endl;
        return 1;
    }
    return 0;
}
