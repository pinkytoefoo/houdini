#pragma once

#include <array>

template<std::size_t Size>
using MoveTable = std::array<Bitboard, Size>;

namespace MoveTables {
    extern const MoveTable<64> KNIGHT_ATTACKS;
    extern const MoveTable<64> KING_MOVES;
};
