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
#include <optional>
#include <chrono>

using namespace std::chrono;

#include "bitboard.hpp"
#include "move_tables.hpp"

enum Piece : std::uint16_t {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    PIECE_NB
};

enum Side : std::uint16_t {
    SIDE_WHITE,
    SIDE_BLACK,
    SIDE_NB,
};

struct FullPiece {
    Side color;
    Piece piece;
};

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

    throw std::logic_error{"invalid char to piece"};
}

struct Move {
    Square from;
    Square to;
};

class Position
{
public:
    Position(const std::string& fen)
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

    [[nodiscard]] bool TryMove(Move move, Piece type) {
        if(!boards_[side_to_move_][type].get(move.from)) {
            return false;
        }

        if(type == KNIGHT) {
            Bitboard attacks = MoveTables::kKnightAttacks[move.from];
            Bitboard valid = attacks & ~Occupancy(side_to_move_);
            // todo: add king check logic
        }

        MakeMove(move, type);
        return true;
    }

    void MakeMove(Move move, Piece type) {
        boards_[side_to_move_][type].set(move.to);
        boards_[side_to_move_][type].set(move.from);
        side_to_move_ = (side_to_move_ == SIDE_WHITE) ? SIDE_BLACK : SIDE_WHITE;
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
                    if(boards_[color][piece].get(square)) {
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
        Bitboard result{ 0ull };

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
        return MoveTables::kKnightAttacks[from] & ~Occupancy(side);
    }
    
    char GetPieceAt(Square square) const {
        static constexpr char tokens[][6] = {
            {'P', 'N', 'B', 'R', 'Q', 'K'},
            {'p', 'n', 'b', 'r', 'q', 'k'}
        };
        for (u16 color{}; color < SIDE_NB; ++color) {
            for (u16 piece{}; piece < PIECE_NB; ++piece) {
                if (boards_[color][piece].get(square)) {
                    return tokens[color][piece];
                }
            }
        }
        return '.';
    }

private:
    Bitboard boards_[SIDE_NB][PIECE_NB]{{}};
    Side side_to_move_{SIDE_WHITE};
    std::optional<Square> en_passant_sq_;
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
                if (bb.get(square)) {
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

__uint128_t g_lehmer64_state = 0x123456789abcdef1ULL;

inline uint64_t lehmer64() {
    g_lehmer64_state *= 0xda942042e4dd58b5ULL;
    return g_lehmer64_state >> 64;
}

inline uint8_t lehmer8() {
    g_lehmer64_state *= 0xda942042e4dd58b5ULL;
    return g_lehmer64_state & 63;
}

int main() {
    Position board("rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/R1BQKBNR");
    board.Print();
    
    constexpr size_t numRnd = 10'000'000;
    std::vector<Bitboard> rndOccupancies;
    std::vector<Square> rndSquares;
    rndOccupancies.reserve(numRnd);
    rndSquares.reserve(numRnd);
    for(size_t i{}; i < numRnd; ++i) {
        rndOccupancies.emplace_back(lehmer64());
        rndSquares.push_back(static_cast<Square>(lehmer8()));
    }
    std::cout << "Starting trials\n";

    auto start = steady_clock::now();
    for(size_t trial{}; trial < numRnd; ++trial) {
        Bitboard attacks = MoveTables::GetRookAttacks(rndSquares[trial], rndOccupancies[trial]);
    }
    auto end = steady_clock::now();

    std::cout << "Time: " << duration_cast<nanoseconds>((end - start) / numRnd) << '\n';
    

    // Print8x8(MoveTables::kKingMoves[D4]);
    // Print8x8(Files::A);
}
