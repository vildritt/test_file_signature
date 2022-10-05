/**
 *  MD5 implementation copied from (BSD-license):
 *     [1] https://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 *  NOTE: boost impl of MD5 in latest versions is detail implementation and may have not RFC compatible results, see https://github.com/boostorg/uuid/issues/111
 *
 *  TODO 1: For asm optimizations check https://github.com/animetosho/md5-optimisation
 */
#include "md5.hpp"

#include <cstdint>
#include <cstring>
#include <cassert>

namespace {

#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define HASHER_MD5_LE_CPU 1
#define HASHER_MD5_BE_CPU 0
#else
#define HASHER_MD5_LE_CPU 0
#define HASHER_MD5_BE_CPU 1
#endif

using MD5_u32 = uint32_t;
using MD5_Size = hash::md5::Size;


constexpr const hash::md5::Size kMD5DataBlockSizeBytes = 64;


struct MD5_State {
    MD5_u32 a = 0x67452301;
    MD5_u32 b = 0xefcdab89;
    MD5_u32 c = 0x98badcfe;
    MD5_u32 d = 0x10325476;
};

} // ns a


// TODO 1: try to replace macroses with constexpr template functions

//-- this part is a modified copy of [1] see header notes


/*
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F(x, y, z)            ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)            ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)            (((x) ^ (y)) ^ (z))
#define H2(x, y, z)           ((x) ^ ((y) ^ (z)))
#define I(x, y, z)            ((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define MD5_ROUND_STEP(f, a, b, c, d, x, t, s) \
    (a) += f((b), (c), (d)) + (x) + (t); \
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
    (a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them in a
 * properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned memory
 * accesses is just an optimization.  Nothing will break if it fails to detect
 * a suitable architecture.
 *
 * Unfortunately, this optimization may be a C strict aliasing rules violation
 * if the caller's data buffer has effective type that cannot be aliased by
 * MD5_u32.  In practice, this problem may occur if these MD5 routines are
 * inlined into a calling function, or with future and dangerously advanced
 * link-time optimizations.  For the time being, keeping these MD5 routines in
 * their own translation unit avoids the problem.
 */
#if HASHER_MD5_LE_CPU

#define GET_SET(n, Type)     *reinterpret_cast<Type *>(ptr + (n)*4)
#define SET(n)               GET_SET((n), MD5_u32)
#define GET(n)               GET_SET((n), const MD5_u32)

#else

#define SET(n) \
    (ctx->buffer[(n)] = \
        (static_cast<MD5_u32>(ptr[(n) * 4 + 0]) <<  0) | \
        (static_cast<MD5_u32>(ptr[(n) * 4 + 1]) <<  8) | \
        (static_cast<MD5_u32>(ptr[(n) * 4 + 2]) << 16) | \
        (static_cast<MD5_u32>(ptr[(n) * 4 + 3]) << 24))
#define GET(n)      (ctx->buffer[(n)])

#endif


namespace hash {
namespace md5 {
namespace detail {

struct HashPrivate {
    Hash* q_ptr;

    MD5_u32 lo = 0;
    MD5_u32 hi = 0;
    MD5_State state;
#if HASHER_MD5_LE_CPU
    std::array<unsigned char, kMD5DataBlockSizeBytes> buffer;
#endif
#if HASHER_MD5_BE_CPU
    std::array<MD5_u32, kMD5DataBlockSize / sizeof(MD5_u32)> buffer;
#endif

    HashPrivate(Hash* q_ptr)
        : q_ptr(q_ptr)
    {
        reinit();
    }


    const void *updateBlocks(const void *data, Size size)
    {
        assert(((size % kMD5DataBlockSizeBytes) == 0) && "size must be multiple of 64");
        assert(data != nullptr && "data buffer must be given");

        const Byte *ptr = reinterpret_cast<const Byte*>(data);

        MD5_State s = state;
        MD5_State prev_s;

        // blocks loop
        do {
            prev_s = s;

            // round 1
            MD5_ROUND_STEP(F, s.a, s.b, s.c, s.d, GET(0),  0xd76aa478,  7)
            MD5_ROUND_STEP(F, s.d, s.a, s.b, s.c, GET(1),  0xe8c7b756, 12)
            MD5_ROUND_STEP(F, s.c, s.d, s.a, s.b, GET(2),  0x242070db, 17)
            MD5_ROUND_STEP(F, s.b, s.c, s.d, s.a, GET(3),  0xc1bdceee, 22)
            MD5_ROUND_STEP(F, s.a, s.b, s.c, s.d, GET(4),  0xf57c0faf,  7)
            MD5_ROUND_STEP(F, s.d, s.a, s.b, s.c, GET(5),  0x4787c62a, 12)
            MD5_ROUND_STEP(F, s.c, s.d, s.a, s.b, GET(6),  0xa8304613, 17)
            MD5_ROUND_STEP(F, s.b, s.c, s.d, s.a, GET(7),  0xfd469501, 22)
            MD5_ROUND_STEP(F, s.a, s.b, s.c, s.d, GET(8),  0x698098d8,  7)
            MD5_ROUND_STEP(F, s.d, s.a, s.b, s.c, GET(9),  0x8b44f7af, 12)
            MD5_ROUND_STEP(F, s.c, s.d, s.a, s.b, GET(10), 0xffff5bb1, 17)
            MD5_ROUND_STEP(F, s.b, s.c, s.d, s.a, GET(11), 0x895cd7be, 22)
            MD5_ROUND_STEP(F, s.a, s.b, s.c, s.d, GET(12), 0x6b901122,  7)
            MD5_ROUND_STEP(F, s.d, s.a, s.b, s.c, GET(13), 0xfd987193, 12)
            MD5_ROUND_STEP(F, s.c, s.d, s.a, s.b, GET(14), 0xa679438e, 17)
            MD5_ROUND_STEP(F, s.b, s.c, s.d, s.a, GET(15), 0x49b40821, 22)

            // round 2
            MD5_ROUND_STEP(G, s.a, s.b, s.c, s.d, GET(1),  0xf61e2562,  5)
            MD5_ROUND_STEP(G, s.d, s.a, s.b, s.c, GET(6),  0xc040b340,  9)
            MD5_ROUND_STEP(G, s.c, s.d, s.a, s.b, GET(11), 0x265e5a51, 14)
            MD5_ROUND_STEP(G, s.b, s.c, s.d, s.a, GET(0),  0xe9b6c7aa, 20)
            MD5_ROUND_STEP(G, s.a, s.b, s.c, s.d, GET(5),  0xd62f105d,  5)
            MD5_ROUND_STEP(G, s.d, s.a, s.b, s.c, GET(10), 0x02441453,  9)
            MD5_ROUND_STEP(G, s.c, s.d, s.a, s.b, GET(15), 0xd8a1e681, 14)
            MD5_ROUND_STEP(G, s.b, s.c, s.d, s.a, GET(4),  0xe7d3fbc8, 20)
            MD5_ROUND_STEP(G, s.a, s.b, s.c, s.d, GET(9),  0x21e1cde6,  5)
            MD5_ROUND_STEP(G, s.d, s.a, s.b, s.c, GET(14), 0xc33707d6,  9)
            MD5_ROUND_STEP(G, s.c, s.d, s.a, s.b, GET(3),  0xf4d50d87, 14)
            MD5_ROUND_STEP(G, s.b, s.c, s.d, s.a, GET(8),  0x455a14ed, 20)
            MD5_ROUND_STEP(G, s.a, s.b, s.c, s.d, GET(13), 0xa9e3e905,  5)
            MD5_ROUND_STEP(G, s.d, s.a, s.b, s.c, GET(2),  0xfcefa3f8,  9)
            MD5_ROUND_STEP(G, s.c, s.d, s.a, s.b, GET(7),  0x676f02d9, 14)
            MD5_ROUND_STEP(G, s.b, s.c, s.d, s.a, GET(12), 0x8d2a4c8a, 20)

            // round 3
            MD5_ROUND_STEP(H,  s.a, s.b, s.c, s.d, GET(5),  0xfffa3942,  4)
            MD5_ROUND_STEP(H2, s.d, s.a, s.b, s.c, GET(8),  0x8771f681, 11)
            MD5_ROUND_STEP(H,  s.c, s.d, s.a, s.b, GET(11), 0x6d9d6122, 16)
            MD5_ROUND_STEP(H2, s.b, s.c, s.d, s.a, GET(14), 0xfde5380c, 23)
            MD5_ROUND_STEP(H,  s.a, s.b, s.c, s.d, GET(1),  0xa4beea44,  4)
            MD5_ROUND_STEP(H2, s.d, s.a, s.b, s.c, GET(4),  0x4bdecfa9, 11)
            MD5_ROUND_STEP(H,  s.c, s.d, s.a, s.b, GET(7),  0xf6bb4b60, 16)
            MD5_ROUND_STEP(H2, s.b, s.c, s.d, s.a, GET(10), 0xbebfbc70, 23)
            MD5_ROUND_STEP(H,  s.a, s.b, s.c, s.d, GET(13), 0x289b7ec6,  4)
            MD5_ROUND_STEP(H2, s.d, s.a, s.b, s.c, GET(0),  0xeaa127fa, 11)
            MD5_ROUND_STEP(H,  s.c, s.d, s.a, s.b, GET(3),  0xd4ef3085, 16)
            MD5_ROUND_STEP(H2, s.b, s.c, s.d, s.a, GET(6),  0x04881d05, 23)
            MD5_ROUND_STEP(H,  s.a, s.b, s.c, s.d, GET(9),  0xd9d4d039,  4)
            MD5_ROUND_STEP(H2, s.d, s.a, s.b, s.c, GET(12), 0xe6db99e5, 11)
            MD5_ROUND_STEP(H,  s.c, s.d, s.a, s.b, GET(15), 0x1fa27cf8, 16)
            MD5_ROUND_STEP(H2, s.b, s.c, s.d, s.a, GET(2),  0xc4ac5665, 23)

            // round 4
            MD5_ROUND_STEP(I, s.a, s.b, s.c, s.d, GET(0),  0xf4292244,  6)
            MD5_ROUND_STEP(I, s.d, s.a, s.b, s.c, GET(7),  0x432aff97, 10)
            MD5_ROUND_STEP(I, s.c, s.d, s.a, s.b, GET(14), 0xab9423a7, 15)
            MD5_ROUND_STEP(I, s.b, s.c, s.d, s.a, GET(5),  0xfc93a039, 21)
            MD5_ROUND_STEP(I, s.a, s.b, s.c, s.d, GET(12), 0x655b59c3,  6)
            MD5_ROUND_STEP(I, s.d, s.a, s.b, s.c, GET(3),  0x8f0ccc92, 10)
            MD5_ROUND_STEP(I, s.c, s.d, s.a, s.b, GET(10), 0xffeff47d, 15)
            MD5_ROUND_STEP(I, s.b, s.c, s.d, s.a, GET(1),  0x85845dd1, 21)
            MD5_ROUND_STEP(I, s.a, s.b, s.c, s.d, GET(8),  0x6fa87e4f,  6)
            MD5_ROUND_STEP(I, s.d, s.a, s.b, s.c, GET(15), 0xfe2ce6e0, 10)
            MD5_ROUND_STEP(I, s.c, s.d, s.a, s.b, GET(6),  0xa3014314, 15)
            MD5_ROUND_STEP(I, s.b, s.c, s.d, s.a, GET(13), 0x4e0811a1, 21)
            MD5_ROUND_STEP(I, s.a, s.b, s.c, s.d, GET(4),  0xf7537e82,  6)
            MD5_ROUND_STEP(I, s.d, s.a, s.b, s.c, GET(11), 0xbd3af235, 10)
            MD5_ROUND_STEP(I, s.c, s.d, s.a, s.b, GET(2),  0x2ad7d2bb, 15)
            MD5_ROUND_STEP(I, s.b, s.c, s.d, s.a, GET(9),  0xeb86d391, 21)

            s.a += prev_s.a;
            s.b += prev_s.b;
            s.c += prev_s.c;
            s.d += prev_s.d;

            ptr += kMD5DataBlockSizeBytes;
            size -= kMD5DataBlockSizeBytes;
        } while (size > 0);

        state = s;

        return ptr;
    }


    void reinit()
    {
        state = MD5_State();

        lo = 0;
        hi = 0;

        std::fill(buffer.begin(), buffer.end(), 0);
    }


    void updateArbitarySizedBuffer(const void *data, MD5_Size size)
    {
        MD5_u32 prev_lo = lo;
        if ((lo = (prev_lo + size) & 0x1fffffff) < prev_lo) {
            hi++;
        }
        hi += size >> 29;

        const unsigned long used = prev_lo & 0x3f;

        if (used) {
            const unsigned long available = kMD5DataBlockSizeBytes - used;

            if (size < available) {
                memcpy(&buffer[used], data, size);
                return;
            }

            memcpy(&buffer[used], data, available);
            data = (const unsigned char *)data + available;
            size -= available;
            updateBlocks(buffer.data(), kMD5DataBlockSizeBytes);
        }

        if (size >= kMD5DataBlockSizeBytes) {
            data = updateBlocks(data, size & ~(unsigned long)0x3f);
            size &= 0x3f;
        }

        memcpy(buffer.data(), data, size);
    }


    static inline void pushU32ToBuffer(Byte *dst, const MD5_u32 src) noexcept
    {
        dst[0] = static_cast<Byte>(src >>  0);
        dst[1] = static_cast<Byte>(src >>  8);
        dst[2] = static_cast<Byte>(src >> 16);
        dst[3] = static_cast<Byte>(src >> 24);
    }


    void finalize(Byte *result)
    {
        unsigned long used = lo & 0x3f;

        buffer[used++] = 0x80;

        unsigned long available = kMD5DataBlockSizeBytes - used;

        if (available < 8) {
            memset(&buffer[used], 0, available);
            updateBlocks(buffer.data(), kMD5DataBlockSizeBytes);
            used = 0;
            available = kMD5DataBlockSizeBytes;
        }

        memset(&buffer[used], 0, available - 8);

        lo <<= 3;

        pushU32ToBuffer(buffer.data() + 56, lo);
        pushU32ToBuffer(buffer.data() + 60, hi);

        updateBlocks(buffer.data(), kMD5DataBlockSizeBytes);

        // state to result

        pushU32ToBuffer(result +  0, state.a);
        pushU32ToBuffer(result +  4, state.b);
        pushU32ToBuffer(result +  8, state.c);
        pushU32ToBuffer(result + 12, state.d);

        // reset

        reinit();
    }

};

// \-- end of copy of [1]


}}} // ns hash::md5::detail


hash::md5::Digest::Digest()
{
    clear();
}


void hash::md5::Digest::clear()
{
    std::fill(binary.begin(), binary.end(), 0);
}


hash::md5::Hash::Hash()
    : d_ptr(new hash::md5::detail::HashPrivate(this))
{

}


hash::md5::Hash::~Hash() noexcept
{
    // for uniq ptr
}


void hash::md5::Hash::reinit() noexcept
{
    d_ptr->reinit();
}


void hash::md5::Hash::process(const Byte *buffer, Size size)
{
    d_ptr->updateArbitarySizedBuffer(buffer, size);
}


hash::md5::Digest hash::md5::Hash::getDigest() const
{
    hash::md5::Digest res;
    d_ptr->finalize(res.binary.data());
    return res;
}
