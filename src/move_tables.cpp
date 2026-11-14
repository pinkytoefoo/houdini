#include "bitboard.hpp"

#include "move_tables.hpp"

namespace MoveTables {
    consteval MoveTable GenerateKnightAttacks() {
        MoveTable attacks{};

        for (int sq = 0; sq < kBoardSize; ++sq) {
            Bitboard b{ 1ull << sq };

            attacks[sq] =
                b.shift<North>().shift<NorthEast>() | 
                b.shift<North>().shift<NorthWest>() |
                b.shift<East>().shift<NorthEast>()  |
                b.shift<West>().shift<NorthWest>()  |
                b.shift<South>().shift<SouthWest>() |
                b.shift<South>().shift<SouthEast>() |
                b.shift<East>().shift<SouthEast>()  |
                b.shift<West>().shift<SouthWest>();
        }

        return attacks;
    }

    consteval MoveTable GenerateKingMoves() {
        MoveTable moves{};

        for (int square{}; square < kBoardSize; ++square) {
            Bitboard b{ 1ull << square };

            moves[square] =
                b.shift<NorthEast>() |
                b.shift<North>() |
                b.shift<NorthWest>() |
                b.shift<West>() |
                b.shift<East>() |
                b.shift<SouthWest>() |
                b.shift<South>() |
                b.shift<SouthEast>();
        }
        return moves;
    }
    
    template<Direction Dir>
    Bitboard GenerateRayAttacks(Square from, Bitboard occupied) {
        Bitboard attacks{};
        Bitboard curr{from};
        
        while(curr) {
            curr = curr.shift<Dir>();
            attacks |= curr; // filter out same side pieces later

            if(curr & occupied) {
                break;
            }
        }

        return attacks;
    }

    template<Direction... Dirs>
    Bitboard GetSlidingAttacks(Square from, Bitboard occupied) {
        return (GenerateRayAttacks<Dirs>(from, occupied) | ...);
    }

    Bitboard GetRookAttacks(Square from, Bitboard occupied) {
        return GetSlidingAttacks<North, East, South, West>(from, occupied);
    }

    Bitboard GetBishopAttacks(Square from, Bitboard occupied) {
        return GetSlidingAttacks<NorthEast, SouthEast, SouthWest, NorthWest>(from, occupied);
    }

    Bitboard GenerateQueenAttacks(Square from, Bitboard occupied) {
        return GetRookAttacks(from, occupied) | GetBishopAttacks(from, occupied);
    }
}

namespace MoveTables {
    const MoveTable kKnightAttacks{GenerateKnightAttacks()};
    const MoveTable kKingMoves{GenerateKingMoves()};
}

