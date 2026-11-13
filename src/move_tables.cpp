#include "constants.hpp"

#include "bitboard.hpp"

namespace {
    consteval MoveTable<64> GenerateKnightAttacks() {
        MoveTable<64> attacks{};

        for (int sq = 0; sq < kBoardSize; ++sq) {
            Bitboard b{ 1ull << sq };

            attacks[sq] =
                (b << 17 & ~Files::A)  | // North-North-East
                (b << 15 & ~Files::H)  | // North-North-West
                (b << 10 & ~Files::AB) | // East-East-North
                (b <<  6 & ~Files::GH) | // West-West-North
                (b >> 17 & ~Files::H)  | // South-South-West
                (b >> 15 & ~Files::A)  | // South-South-East
                (b >> 10 & ~Files::GH) | // West-West-South
                (b >>  6 & ~Files::AB);  // East-East-South
        }

        return attacks;
    }

    consteval MoveTable<64> GenerateKingMoves() {
        MoveTable<64> moves{};

        for (int square{}; square < kBoardSize; ++square) {
            const Bitboard b = 1ULL << square;

            moves[square] =
                (b << 7 & ~Files::H) | // Up-Right
                (b << 8)                  | // Up
                (b << 9 & ~Files::A) | // Up-Left
                (b << 1 & ~Files::A) | // Left
                (b >> 1 & ~Files::H) | // Right
                (b >> 7 & ~Files::A) | // Down-Left
                (b >> 8)                  | // Down
                (b >> 9 & ~Files::H);  // Down-Light
        }
        return moves;
    }
}

namespace MoveTables {
    const MoveTable<64> KNIGHT_ATTACKS{GenerateKnightAttacks()};
    const MoveTable<64> KING_MOVES{GenerateKingMoves()};
}

