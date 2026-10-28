#pragma once

#include <cstdint>
#include <array>

using Bitboard = std::uint64_t;
using u16 = std::uint16_t;
using MoveTable = std::array<Bitboard, 64>; // used for knight and king moves
