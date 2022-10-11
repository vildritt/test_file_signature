#include "file_stream_writer.hpp"


ss::FileStreamDigestWriter::FileStreamDigestWriter(const std::string &outputFilePath)
    : StreamDigestWriter(nullptr)
    , m_fileOutputStream(outputFilePath, std::ios_base::trunc)
{
    if (!m_fileOutputStream.is_open()) {
        throw std::runtime_error("failed to open output file: " + outputFilePath);
    }

    setOutputStream(&m_fileOutputStream);
}
