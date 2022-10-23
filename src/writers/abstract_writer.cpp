#include "abstract_writer.hpp"


void ss::AstractDigestWriter::write(const tools::hash::Digest &digest)
{
    doWrite(digest);
}


void ss::AstractDigestWriter::flush()
{
    doFlush();
}


void ss::AstractDigestWriter::doFlush()
{
    // default: nothing
}
