#ifndef SS_STREAM_WRITERS_FILE_STREAM_WRITER_H
#define SS_STREAM_WRITERS_FILE_STREAM_WRITER_H
#pragma once

#include "writers/stream_writer.hpp"

#include <fstream>
#include <string>


namespace ss {


class FileStreamDigestWriter : public StreamDigestWriter
{
public:
    FileStreamDigestWriter(const std::string& outputFilePath);
private:
    std::ofstream m_fileOutputStream;
};


} // ns ss


#endif // SS_STREAM_WRITERS_FILE_STREAM_WRITER_H
