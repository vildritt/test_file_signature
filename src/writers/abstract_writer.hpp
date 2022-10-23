#ifndef SS_WRITERS_ABSTRACT_WRITER_H
#define SS_WRITERS_ABSTRACT_WRITER_H
#pragma once

#include <memory>

#include <tools/hash/digest.hpp>


namespace ss {


class AstractDigestWriter {
public:
    virtual ~AstractDigestWriter() {}

    /// push digest to output
    void write(const tools::hash::Digest& digest);
    /// do any flushing for buffered outputs
    void flush();
private:
    virtual void doWrite(const tools::hash::Digest& digest) = 0;
    virtual void doFlush();
};


using DigestWriterPtr = std::shared_ptr<AstractDigestWriter>;


} // ns ss


#endif // SS_WRITERS_ABSTRACT_WRITER_H
