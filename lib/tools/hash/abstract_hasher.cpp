#include <tools/hash/abstract_hasher.hpp>


void tools::hash::AbstractHasher::initialize()
{
    doInitialize();
}


void tools::hash::AbstractHasher::process(const std::string_view &buffer)
{
    doProcess(buffer);
}


tools::hash::Digest tools::hash::AbstractHasher::finalize()
{
    return doFinalize();
}


tools::hash::AbstractHasher::AbstractHasher()
{

}

tools::hash::Digest tools::hash::AbstractHasher::hash(const std::string_view &buffer)
{
    initialize();
    process(buffer);
    return finalize();
}


size_t tools::hash::AbstractHasherFactory::digestSize()
{
    return doGetDigestSize();
}


size_t tools::hash::AbstractHasherFactory::doGetDigestSize()
{
    if (m_digestSize > 0) {
        return m_digestSize;
    }

    // est hasher digest size by run by def

    auto hasher = create();

    std::array<char, 10> test{};
    const auto digest = hasher->hash(std::string_view(test.data(), test.size()));
    m_digestSize =
            std::max<tools::SizeBytes>(
                1,
                digest.binary.size());

    return m_digestSize;
}


tools::hash::HasherPtr tools::hash::AbstractHasherFactory::create()
{
    return doCreate();
}
