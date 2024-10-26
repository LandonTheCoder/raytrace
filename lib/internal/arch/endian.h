#pragma once

// In C++23 we have a byteswap function
#ifdef __cpp_lib_endian
#include <bit>

#if std::endian::native == std::endian::little
#define IS_LITTLE_ENDIAN 1
#elif std::endian::native == std::endian::big
#define IS_BIG_ENDIAN 1
#endif // std::endian::native

#elif defined __BIG_ENDIAN__
#define IS_BIG_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define IS_BIG_ENDIAN 1
#elif defined __LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN 1
#endif // __cpp_lib_endian

#ifdef IS_LITTLE_ENDIAN
#define TO_LE16(i) i
#define TO_LE32(i) i
#elif IS_BIG_ENDIAN
// Platform-specific
#ifdef __cpp_lib_byteswap
#define TO_LE16(i) std::byteswap(i)
#define TO_LE32(i) std::byteswap(i)

#elif defined __linux__ || defined __FreeBSD__
// Linux and BSD use endian.h
#include <endian.h>

#define TO_LE16(i) htole16(i)
#define TO_LE32(i) htole32(i)

#elif define __GNUC__
// Use GCC compiler intrinsics
#define TO_LE16(i) __builtin_bswap16(i)
#define TO_LE32(i) __builtin_bswap32(i)
#else
// Bad fallback for byteswap.
#define TO_LE16(i) ((i>>8) | (i<<8))
#define TO_LE32(i) (((num>>24) & 0xff) | \
                    ((num<<8) & 0xff0000) | \
                    ((num>>8) & 0xff00) | \
                    ((num<<24) & 0xff000000))

#endif // defined __linux__

#else
// Assume little-endian? May not work.
#warning "Endian not detected, assuming little-endian."
#define TO_LE16(i) htole16(i)
#define TO_LE32(i) htole32(i)

#endif // IS_LITTLE_ENDIAN
