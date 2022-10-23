#ifndef SS_STREAM_WRITERS_STREAM_WRITER_H
#define SS_STREAM_WRITERS_STREAM_WRITER_H
#pragma once

#include <ostream>
#include <string>

#include "writers/abstract_writer.hpp"


namespace ss {


class StreamDigestWriter : public ss::AstractDigestWriter
{
public:
    StreamDigestWriter(std::ostream* outputStream);
    void setOutputStream(std::ostream* outputStream);
private:
    void doWrite(const tools::hash::Digest& digest) override;
    void doFlush() override;

    std::ostream* m_outputStream = nullptr;
};

} // ns ss


#endif // SS_STREAM_WRITERS_STREAM_WRITER_H
