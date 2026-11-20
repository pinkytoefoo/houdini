#pragma once

#include <array>
#include <cstdint>

#include "bitboard.hpp"

static constexpr std::size_t kBoardSize = 64;
using MoveTable = std::array<Bitboard, kBoardSize>;

namespace MoveTables {
    consteval MoveTable GenerateKnightAttacks();

    Bitboard GetRookAttacks(Square from, Bitboard occupied);
    Bitboard GetBishopAttacks(Square from, Bitboard occupied);
    Bitboard GenerateQueenAttacks(Square from, Bitboard occupied);

    extern const MoveTable kKnightAttacks;
    extern const MoveTable kKingMoves;
};
