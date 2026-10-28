#pragma once

#include "aliases.hpp"

namespace FilesMasks {
    inline constexpr Bitboard A  = 0x0101010101010101ULL;
    inline constexpr Bitboard AB = A | (A << 1);
    inline constexpr Bitboard H  = 0x8080808080808080ULL;
    inline constexpr Bitboard GH = H | (H >> 1);
}

static constexpr size_t kBoardSize = 64ul;

