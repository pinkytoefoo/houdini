#include "constants.hpp"

#include "move_tables.hpp"

namespace {
    consteval MoveTable GenerateKnightAttacks() {
        MoveTable attacks{};

        for (int sq = 0; sq < kBoardSize; ++sq) {
            Bitboard b = 1ull << sq;

            attacks[sq] =
                (b << 17 & ~FilesMasks::A)  | // North-North-East
                (b << 15 & ~FilesMasks::H)  | // North-North-West
                (b << 10 & ~FilesMasks::AB) | // East-East-North
                (b <<  6 & ~FilesMasks::GH) | // West-West-North
                (b >> 17 & ~FilesMasks::H)  | // South-South-West
                (b >> 15 & ~FilesMasks::A)  | // South-South-East
                (b >> 10 & ~FilesMasks::GH) | // West-West-South
                (b >>  6 & ~FilesMasks::AB);  // East-East-South
        }

        return attacks;
    }

    consteval MoveTable GenerateKingMoves() {
        MoveTable moves{};

        for (int square{}; square < kBoardSize; ++square) {
            const Bitboard b = 1ULL << square;

            moves[square] =
                (b << 7 & ~FilesMasks::H) | // Up-Right
                (b << 8)                  | // Up
                (b << 9 & ~FilesMasks::A) | // Up-Left
                (b << 1 & ~FilesMasks::A) | // Left
                (b >> 1 & ~FilesMasks::H) | // Right
                (b >> 7 & ~FilesMasks::A) | // Down-Left
                (b >> 8)                  | // Down
                (b >> 9 & ~FilesMasks::H);  // Down-Light
        }
        return moves;
    }
}

namespace MoveTables {
    const MoveTable KNIGHT_ATTACKS{GenerateKnightAttacks()};
    const MoveTable KING_MOVES{GenerateKingMoves()};
}

