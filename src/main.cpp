#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <vector>

#include "usage.hpp"
#include "md5.hpp"


using MD5_Size = uint64_t;
using Digest = std::vector<unsigned char>;

static constexpr const MD5_Size kKiloBytes = 1024;
static constexpr const MD5_Size kMegaBytes = 1024 * kKiloBytes;

#ifndef _NDEBUG
static constexpr const MD5_Size kMinBlockSizeBytes = 1;
#else
static constexpr const MD5_Size kMinBlockSizeBytes = 512;
#endif
static constexpr const MD5_Size kMaxBlockSizeBytes = 10 * kMegaBytes;


struct Options {
    static constexpr const MD5_Size kDefaultBlockSize = 1 * kMegaBytes;

    std::string inputFilePath;
    std::string outputFilePath;
    MD5_Size blockSizeBytes = kDefaultBlockSize;
};


MD5_Size parseBlockSize(const std::string& blockSizeText)
{
    size_t idx = 0;
    MD5_Size res = std::stoull(blockSizeText, &idx, 10);
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


Digest blockDigest(const char* data, MD5_Size size)
{
    hash::md5::Hash hasher;
    hasher.process(reinterpret_cast<const hash::md5::Byte*>(data), size);
    const auto digest = hasher.getDigest();

    Digest result;
    result.resize(hash::md5::Digest::kSize);
    std::copy(digest.binary.begin(), digest.binary.end(), result.begin());

    return result;
}


void getSignatures(const Options& opts)
{
    assert(opts.blockSizeBytes > 0 && "block size must be positive");

    const std::filesystem::path finp(opts.inputFilePath);
    if (!std::filesystem::exists(finp)) {
        throw std::runtime_error("input file not exists: " + opts.inputFilePath);
    }

    const auto size = std::filesystem::file_size(finp);
    const auto blockSize = opts.blockSizeBytes;
    auto blockCount = size / blockSize;
    const auto lastBlockRealSize = size - blockCount * blockSize;
    const bool needToFillLastBlock = lastBlockRealSize > 0;
    if (size == 0 || needToFillLastBlock) {
        blockCount++;
    }

    std::ifstream fin(opts.inputFilePath, std::ios_base::binary);
    if (fin.bad()) {
        throw std::runtime_error("failed to open in file: " + opts.inputFilePath);
    }

    std::ostream* sout = &std::cout;
    std::unique_ptr<std::ofstream> fout;

    if (!opts.outputFilePath.empty()) {
        fout = std::make_unique<std::ofstream>(opts.outputFilePath, std::ios_base::trunc);
        sout = fout.get();

        if (!fout->is_open()) {
            throw std::runtime_error("failed to open out file: " + opts.inputFilePath);
        }

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
