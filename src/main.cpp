#include <iostream>
#include <bitset>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <bit>
#include <cassert>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <array>
#include <ranges>
#include <string_view>

#include "constants.hpp"

enum Square : u16 {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NONE,
    SQUARE_NB = 64
};

enum Piece : u16 {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    PIECE_NB
};

enum Side : u16 {
    SIDE_WHITE,
    SIDE_BLACK,
    SIDE_NB,
};

struct FullPiece {
    Side color;
    Piece piece;
};

namespace BitboardUtils {
   inline void set_bit(Bitboard& bb, Square square) {
        bb |= Bitboard{1} << square;
    }

    inline void clear_bit(Bitboard& bb, Square square) {
        bb &= ~(Bitboard{1} << square);
    }

    inline bool get_bit(Bitboard bb, Square square) {
        return ((bb >> square) & Bitboard{1}) != 0;
    }

    inline int count_bits(Bitboard bb) {
        return std::popcount(bb); // C++20
    }

    inline Square pop_lsb(Bitboard& bb) {
        assert(bb != 0);
        const auto square = std::countr_zero(bb);
        bb &= bb - 1;
        return static_cast<Square>(square);
    }

    inline Bitboard isolate_bit_from_board(Bitboard bb, Square square) {
        return bb & (Bitboard{1} << square);
    }
}

constexpr FullPiece CharToPiece(char ch) {
    switch (ch) {
        case 'P': return {SIDE_WHITE, PAWN};
        case 'N': return {SIDE_WHITE, KNIGHT};
        case 'B': return {SIDE_WHITE, BISHOP};
        case 'R': return {SIDE_WHITE, ROOK};
        case 'Q': return {SIDE_WHITE, QUEEN};
        case 'K': return {SIDE_WHITE, KING};

        case 'p': return {SIDE_BLACK, PAWN};
        case 'n': return {SIDE_BLACK, KNIGHT};
        case 'b': return {SIDE_BLACK, BISHOP};
        case 'r': return {SIDE_BLACK, ROOK};
        case 'q': return {SIDE_BLACK, QUEEN};
        case 'k': return {SIDE_BLACK, KING};
    }

    throw "invalid piece character";
}

namespace MoveTables {
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
                (b << 7 & ~FilesMasks::H) | // up-right
                (b << 8)                  | // up
                (b << 9 & ~FilesMasks::A) | // up-left
                (b << 1 & ~FilesMasks::A) | // left
                (b >> 1 & ~FilesMasks::H) | // right
                (b >> 7 & ~FilesMasks::A) | // down-left
                (b >> 8)                  | // down
                (b >> 9 & ~FilesMasks::H);  // down-right
        }
        return moves;
    }

    constexpr MoveTable KNIGHT_ATTACKS{GenerateKnightAttacks()};
    constexpr MoveTable KING_MOVES{GenerateKingMoves()};
};

class Board
{
public:
    Board(const std::string& fen)
    {
        LoadPositionFromFen(fen);
    }

    void LoadPositionFromFen(const std::string& fen) {
        int file = 0;
        int rank = 7;
        for(auto c : fen) {
            if(c == '/') {
                file = 0;
                --rank;
            } else if(std::isdigit(c)) {
                file += c - '0';
            } else {
                const auto cp = CharToPiece(c);
                boards_[cp.color][cp.piece] |= Bitboard{1} << (rank * 8 + file);
                ++file;
            }
        }
    }

    [[nodiscard]] bool TryMove(const std::string& move) {
        switch(move[0]) {
            case 'N':
                break;
            case 'n':
                break;
            default:
                // boards_[toMove_][PAWN];
                break;
        }
    }

    void Print() {
        static constexpr char tokens[][6] = {
            {'P', 'N', 'B', 'R', 'Q', 'K'},
            {'p', 'n', 'b', 'r', 'q', 'k'}
        };

        std::cout << "\n\n  +---+---+---+---+---+---+---+---+\n";

        for (u16 rank{8}; rank-- > 0; ) {
            std::cout << rank + 1 << " | ";

            for (u16 file = 0; file < 8; ++file) {
                Square square = static_cast<Square>(rank * 8 + file);
                char piece_rep = '.';

                for (u16 color{}; color < SIDE_NB; ++color) {
                    for(u16 piece{}; piece < PIECE_NB; ++piece)
                    if (BitboardUtils::get_bit(boards_[color][piece], square)) {
                        piece_rep = tokens[color][piece];
                        break;
                    }
                }

                std::cout << piece_rep << " | ";
            }
            std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
        }
        std::cout << "    a   b   c   d   e   f   g   h\n\n";
    }

    [[nodiscard]] Bitboard Occupancy(Side side) const {
        Bitboard result = 0;

        for(short piece{}; piece < PIECE_NB; ++piece) {
            result |= boards_[side][piece];
        }

        return result;
    }

    [[nodiscard]] Bitboard Occupancy() const {
        return Occupancy(SIDE_WHITE) | Occupancy(SIDE_BLACK);
    }

    // todo: check if there is really a knight on from square
    Bitboard KnightPsuedoMoves(Square from, Side side) {
        return MoveTables::KNIGHT_ATTACKS[from] & ~Occupancy(side);
    }
    
    char GetPieceAt(Square square) const {
        static constexpr char tokens[][6] = {
            {'P', 'N', 'B', 'R', 'Q', 'K'},
            {'p', 'n', 'b', 'r', 'q', 'k'}
        };
        for (u16 color{}; color < SIDE_NB; ++color) {
            for (u16 piece{}; piece < PIECE_NB; ++piece) {
                if (((boards_[color][piece] >> square) & 1) != 0) {
                    return tokens[color][piece];
                }
            }
        }
        return '.';
    }

private:
    Bitboard boards_[SIDE_NB][PIECE_NB]{{}};
    Side toMove_{SIDE_WHITE};
};

void Print8x8(Bitboard bb) {
    static constexpr char tokens[][6] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'}
    };

    std::cout << "\n\n  +---+---+---+---+---+---+---+---+\n";

    for (u16 rank = 8; rank-- > 0;) {
        std::cout << rank + 1 << " | ";

        for (u16 file = 0; file < 8; ++file) {
            Square square = static_cast<Square>(rank * 8 + file);
            char piece_rep = '.';

            for (u16 color{}; color < SIDE_NB; ++color) {
                for(u16 piece{}; piece < PIECE_NB; ++piece)
                if (BitboardUtils::get_bit(bb, square)) {
                    piece_rep = 'O';
                    break;
                }
            }

            std::cout << piece_rep << " | ";
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "    a   b   c   d   e   f   g   h\n\n";
}

int main() {
    Board board("rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/R1BQKBNR");
    board.Print();

    Print8x8(MoveTables::KING_MOVES[D4]);
    Print8x8(FilesMasks::A);
}