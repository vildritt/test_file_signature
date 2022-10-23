#ifndef SS_READER_H
#define SS_READER_H
#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <functional>

#include "slices_scheme.hpp"


namespace ss {


/**
 * @brief Buffered file block reader
 */
class FileBlockReader {
public:
    FileBlockReader(FileBlockReader&& inst) = delete;
    FileBlockReader& operator=(const FileBlockReader& inst) = delete;
    FileBlockReader& operator=(FileBlockReader&& inst) = delete;
    ~FileBlockReader() = default;


    /**
     * @param inputFilePath       - input file path
     * @param fileSlicesScheme    - slices setup @see SlicesScheme
     * @param readBufferSizeBytes - buffer size hint. 0 => autochoose
     */
    FileBlockReader(const std::string& inputFilePath,
                const FileSlicesScheme& fileSlicesScheme,
                const SizeBytes readBufferSizeBytes = 0);
    FileBlockReader(const FileBlockReader& inst);

    /**
     * @brief read block into internal buffer and return view to it
     * @param blockIndex - zero based block index
     * @return view to internal buffer
     */
    std::string_view readSingleBlock(size_t blockIndex);

    /**
     * @brief copy of original slices setup
     */
    const FileSlicesScheme& fileSlicesScheme() const;

private:
    const std::string m_filePath;
    const FileSlicesScheme m_fileSlicesScheme;
    std::ifstream m_fileStream;

    std::vector<char> m_readBuffer;
    std::vector<char> m_blockBuffer;

    ss::SizeBytes m_currentFilePosition = 0;
};


using FileBlockReaderPtr = std::shared_ptr<FileBlockReader>;


class FileBlockReaderFactory {
public:
    virtual ~FileBlockReaderFactory() {}
    FileBlockReaderPtr create();
private:
    virtual FileBlockReaderPtr doCreate() = 0;
};


using FileBlockReaderFactoryPtr = std::shared_ptr<FileBlockReaderFactory>;


class FileBlockReaderFactoryDelegate : public FileBlockReaderFactory {
public:
    using Delegate = std::function<FileBlockReaderPtr(void)>;
    FileBlockReaderFactoryDelegate(Delegate delegate);
private:
    FileBlockReaderPtr doCreate() override;
    Delegate m_delegate;
};


} // ns ss


#endif // SS_READER_H
