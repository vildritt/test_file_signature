#ifndef SS_READER_H
#define SS_READER_H
#pragma once

#include <string>
#include <fstream>
#include <vector>


#include "slices_scheme.hpp"

namespace ss {

class BlockReader {
public:
    BlockReader(const std::string& filePath, const SlicesScheme& slices, const SizeBytes readBufferSize = 0);

    BlockReader(const BlockReader& inst);

    BlockReader(BlockReader&& inst) = delete;
    BlockReader& operator=(const BlockReader& inst) = delete;
    BlockReader& operator=(BlockReader&& inst) = delete;
    ~BlockReader() = default;


    std::string_view readBlock(size_t blockIndex);
    const SlicesScheme& slices() const;

private:
    const std::string m_filePath;
    const SlicesScheme m_slices;
    std::ifstream m_ifs;
    std::vector<char> m_blockBuffer;
    std::vector<char> m_readBuffer;
    ss::SizeBytes m_lastPos = 0;
};



} // ns ss

#endif // SS_READER_H
