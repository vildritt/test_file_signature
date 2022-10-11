#include "stream_writer.hpp"


ss::StreamDigestWriter::StreamDigestWriter(std::ostream *outputStream)
    : m_outputStream(outputStream)
{

}


void ss::StreamDigestWriter::setOutputStream(std::ostream *outputStream)
{
    m_outputStream = outputStream;
}


void ss::StreamDigestWriter::doWrite(const tools::hash::Digest &digest)
{
    *m_outputStream << digest;
}


void ss::StreamDigestWriter::doFlush()
{
    *m_outputStream << std::flush;
}


