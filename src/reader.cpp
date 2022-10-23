#include "reader.hpp"

#include "types.hpp"


ss::FileBlockReader::FileBlockReader(const std::string &inputFilePath, const FileSlicesScheme &fileSlicesScheme, const ss::SizeBytes readBufferSizeBytes)
    : m_filePath(inputFilePath)
    , m_fileSlicesScheme(fileSlicesScheme)
{
    if (readBufferSizeBytes > 0) {
        m_readBuffer.resize(readBufferSizeBytes);
        m_fileStream.rdbuf()->pubsetbuf(m_readBuffer.data(), readBufferSizeBytes);
    }

    m_fileStream.open(m_filePath, std::ios_base::binary);
    if (m_fileStream.bad()) {
        throw std::runtime_error("failed to open in file: " + inputFilePath);
    }

    m_blockBuffer.resize(m_fileSlicesScheme.blockSizeBytes);
}


ss::FileBlockReader::FileBlockReader(const FileBlockReader &inst)
    : FileBlockReader(inst.m_filePath, inst.m_fileSlicesScheme, inst.m_readBuffer.size())
{}


std::string_view ss::FileBlockReader::readSingleBlock(size_t blockIndex)
{
    char* blockBuffer = m_blockBuffer.data();
    const bool isLastBlock = (blockIndex == m_fileSlicesScheme.lastBlock.index);

    const ss::SizeBytes readPosition = m_fileSlicesScheme.blockSizeBytes * blockIndex;

    // avoid ssystem calls due perf
    if (readPosition != m_currentFilePosition) {
        m_fileStream.seekg(readPosition, std::ios_base::beg);
        m_currentFilePosition = readPosition;
    }

    if (isLastBlock && m_fileSlicesScheme.lastBlock.needToFillUpWithZeros) {
        if (m_fileSlicesScheme.lastBlock.realSizeBytes > 0) {
            if (!m_fileStream.read(blockBuffer, m_fileSlicesScheme.lastBlock.realSizeBytes).good()) {
                throw std::runtime_error("block read error");
            }
            m_currentFilePosition += m_fileSlicesScheme.lastBlock.realSizeBytes;
        }
        std::fill(m_blockBuffer.begin() + m_fileSlicesScheme.lastBlock.realSizeBytes, m_blockBuffer.end(), 0);
    } else {
        if (!m_fileStream.read(blockBuffer, m_fileSlicesScheme.blockSizeBytes).good()) {
            throw std::runtime_error("block read error");
        }
        m_currentFilePosition += m_fileSlicesScheme.blockSizeBytes;
    }
    return std::string_view(blockBuffer, m_fileSlicesScheme.blockSizeBytes);
}


const ss::FileSlicesScheme &ss::FileBlockReader::fileSlicesScheme() const
{
    return m_fileSlicesScheme;
}


ss::FileBlockReaderPtr ss::FileBlockReaderFactory::create()
{
    return doCreate();
}


ss::FileBlockReaderFactoryDelegate::FileBlockReaderFactoryDelegate(Delegate delegate)
    : m_delegate(delegate)
{
}


ss::FileBlockReaderPtr ss::FileBlockReaderFactoryDelegate::doCreate()
{
    return m_delegate();
}
