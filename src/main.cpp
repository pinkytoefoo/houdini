#include <iostream>
#include <bitset>
#include <cstdint>
#include <unordered_map>

using Bitboard = std::uint64_t;

namespace BitboardUtils {
    inline void set_bit(Bitboard& bb, int square) {
        bb |= (1ull << square);
    }
    inline void clear_bit(Bitboard& bb, int square) {
        bb &= ~(1ull << square);
    }
    inline bool get_bit(Bitboard& bb, int square) {
        return (bb >> square) & 1ull;
    }

    inline int count_bits(Bitboard bb) {
        return __builtin_popcountll(bb);
    }

    inline int pop_lsb(Bitboard& bb) {
        int index = __builtin_ctzll(bb);
        bb &= (bb - 1);
        return index;
    }
}

enum Square {
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

enum BitboardType {
    WhitePawns,
    WhiteKnights,
    WhiteBishops,
    WhiteRooks,
    WhiteQueens,
    WhiteKing,

    BlackPawns,
    BlackKnights,
    BlackBishops,
    BlackRooks,
    BlackQueens,
    BlackKing,
    Count
};

class Board
{
public:
    Board(const std::string& fen)
    {
        LoadPositionFromFen(fen);
    }

    void LoadPositionFromFen(const std::string& fen) {
        static const std::unordered_map<char, BitboardType> charToPiece = {
            {'P', BitboardType::WhitePawns},
            {'N', BitboardType::WhiteKnights},
            {'B', BitboardType::WhiteBishops},
            {'R', BitboardType::WhiteRooks},
            {'Q', BitboardType::WhiteQueens},
            {'K', BitboardType::WhiteKing},

            {'p', BitboardType::BlackPawns},
            {'n', BitboardType::BlackKnights},
            {'b', BitboardType::BlackBishops},
            {'r', BitboardType::BlackRooks},
            {'q', BitboardType::BlackQueens},
            {'k', BitboardType::BlackKing},
        };
        int file = 0;
        int rank = 7;
        for(auto c : fen) {
            if(c == '/') {
                file = 0;
                --rank;
            } else if(isdigit(c)) {
                file += c - '0';
            } else {
                boards_[charToPiece.at(c)] |= (1ull << (rank * 8 + file));
                ++file;
            }
        }
    }

    [[nodiscard]] bool TryMove(Square square) {
        
    }

    void Print() {
        static constexpr char tokens[] = {
            'P', 'N', 'B', 'R', 'Q', 'K',
            'p', 'n', 'b', 'r', 'q', 'k'
        };

        for (int rank = 7; rank >= 0; --rank) {
            std::cout << rank + 1 << " | ";

            for (int file = 0; file < 8; ++file) {
                const int square = rank * 8 + file;
                char piece_rep = '.';

                for (int i = 0; i < BitboardType::Count; ++i) {
                    if (BitboardUtils::get_bit(boards_[i], square)) {
                        piece_rep = tokens[i];
                        break;
                    }
                }

                std::cout << piece_rep << " | ";
            }

            std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
        }

        std::cout << "    a   b   c   d   e   f   g   h\n\n";
    }

private:
    Bitboard boards_[BitboardType::Count]{};

};

int main() {
    Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    board.Print();
}