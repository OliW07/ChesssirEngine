#pragma once

#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace ChessEngine {
namespace Utils {

// Count trailing zeros in 64-bit integer
inline int ctz64(uint64_t value) {
#ifdef _MSC_VER
    unsigned long index;
    if (value == 0) return 64;
    _BitScanForward64(&index, value);
    return static_cast<int>(index);
#else
    return __builtin_ctzll(value);
#endif
}

// Count leading zeros in 64-bit integer  
inline int clz64(uint64_t value) {
#ifdef _MSC_VER
    unsigned long index;
    if (value == 0) return 64;
    _BitScanReverse64(&index, value);
    return 63 - static_cast<int>(index);
#else
    return __builtin_clzll(value);
#endif
}

// Count set bits (population count) in 64-bit integer
inline int popcount64(uint64_t value) {
#ifdef _MSC_VER
    return __popcnt64(value);
#else
    return __builtin_popcountll(value);
#endif
}

} // namespace Utils
} // namespace ChessEngine