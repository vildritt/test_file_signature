#include "reader.hpp"
#include "types.hpp"

ss::BlockReader::BlockReader(const std::string &filePath, const SlicesScheme &slices)
    : m_filePath(filePath)
    , m_slices(slices)
{
    m_ifs.open(m_filePath, std::ios_base::binary);
    if (m_ifs.bad()) {
        throw std::runtime_error("failed to open in file: " + filePath);
    }
    m_blockBuffer.resize(m_slices.blockCount);
}


ss::BlockReader::BlockReader(const BlockReader &inst)
    : BlockReader(inst.m_filePath, inst.m_slices)
{}


std::string_view ss::BlockReader::readBlock(size_t blockIndex)
{
    char* buffer = m_blockBuffer.data();
    const bool islast = (blockIndex == m_slices.lastBlockIndex);

    const ss::SizeBytes pos = static_cast<SizeBytes>(m_slices.blockSize) * blockIndex;
    m_ifs.seekg(pos, std::ios_base::seekdir::_S_beg);

    if (islast && m_slices.needToFillUplastBlock) {
        m_ifs.read(buffer, m_slices.lastBlockRealSize);
        std::fill(m_blockBuffer.begin() + m_slices.lastBlockRealSize, m_blockBuffer.end(), 0);
    } else {
        m_ifs.read(buffer, m_slices.blockSize);
    }
    return std::string_view(buffer, m_slices.blockSize);
}


const ss::SlicesScheme &ss::BlockReader::slices() const
{
    return m_slices;
}