#include <iostream>
#include <fstream>
#include <filesystem>

#include "types.hpp"
#include "consts.hpp"
#include "misc.hpp"
#include "md5.hpp"
#include "reader.hpp"
#include "digest.hpp"


ss::Digest blockDigest(const std::string_view& buffer)
{
    hash::md5::Hash hasher;
    hasher.process(reinterpret_cast<const hash::md5::Byte*>(buffer.data()), buffer.size());
    const auto digest = hasher.getDigest();

    return ss::Digest(digest.binary.data(), hash::md5::Digest::kSize);
}


void getSignatures(const misc::Options& opts)
{
    const std::filesystem::path finp(opts.inputFilePath);
    if (!std::filesystem::exists(finp)) {
        throw std::runtime_error("input file not exists: " + opts.inputFilePath);
    }

    const ss::SlicesScheme slices(std::filesystem::file_size(finp), opts.blockSizeBytes);

    std::ostream* sout = &std::cout;
    std::unique_ptr<std::ofstream> fout;

    if (!opts.outputFilePath.empty()) {
        fout = std::make_unique<std::ofstream>(opts.outputFilePath, std::ios_base::trunc);
        sout = fout.get();

        if (!fout->is_open()) {
            throw std::runtime_error("failed to open out file: " + opts.inputFilePath);
        }
    }

    ss::BlockReader reader(opts.inputFilePath, slices);

    for(size_t i = 0; i < slices.blockCount; ++i) {
        const ss::Digest digest = blockDigest(reader.readBlock(i));
        *sout << digest;
    }
}


int main(int argc, const char* argv[])
{
    try {
        const auto opts = misc::parseCliParameters(argc, argv);
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
