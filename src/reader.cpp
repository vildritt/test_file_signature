#include "reader.hpp"

#include "types.hpp"
#include "consts.hpp"


ss::BlockReader::BlockReader(const std::string &filePath, const SlicesScheme &slices, const ss::SizeBytes readBufferSize)
    : m_filePath(filePath)
    , m_slices(slices)
{
    if (readBufferSize > 0) {
        m_readBuffer.resize(readBufferSize);
        m_ifs.rdbuf()->pubsetbuf(m_readBuffer.data(), readBufferSize);
    }

    m_ifs.open(m_filePath, std::ios_base::binary);
    if (m_ifs.bad()) {
        throw std::runtime_error("failed to open in file: " + filePath);
    }
    m_blockBuffer.resize(m_slices.blockSize);
}


ss::BlockReader::BlockReader(const BlockReader &inst)
    : BlockReader(inst.m_filePath, inst.m_slices, inst.m_readBuffer.size())
{}


std::string_view ss::BlockReader::readBlock(size_t blockIndex)
{
    char* buffer = m_blockBuffer.data();
    const bool islast = (blockIndex == m_slices.lastBlockIndex);

    const ss::SizeBytes pos = static_cast<SizeBytes>(m_slices.blockSize) * blockIndex;
    if (pos != m_ifs.tellg()) {
        m_ifs.seekg(pos, std::ios_base::beg);
    }

    if (islast && m_slices.needToFillUplastBlock) {
        if (m_slices.lastBlockRealSize > 0) {
            m_ifs.read(buffer, m_slices.lastBlockRealSize);
        }
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
