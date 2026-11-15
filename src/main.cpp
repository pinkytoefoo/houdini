#include "types.hpp"
#include <iostream>
#include <bitset>
#include <cstdint>
#include <limits>
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

class Bitboard;
void Print8x8(Bitboard bb);

enum Piece : std::uint16_t {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    PIECE_NB
};

// todo: add square weights and game stage (early, mid, late game weights)
int PieceVal(Piece type) {
    switch(type) {
        case PAWN: return 1;
        case KNIGHT: case BISHOP: return 3;
        case ROOK: return 5;
        case QUEEN: return 9;
        default:
            throw std::logic_error{"piece values are only assigned to valued pieces"};
    }
}

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
    Piece type;
};

struct UndoInfo {
    FullPiece captured_piece;
    Square square;
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

        MakeMove(move);
        return true;
    }

    std::optional<UndoInfo> MakeMove(Move move) {
        std::optional<UndoInfo> undo;
        Side us = side_to_move_;
        Side them = side_to_move_ == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
        
        for(int type{}; type < QUEEN; ++type) {
            if(boards_[them][static_cast<Piece>(type)].get(move.to)) {
                undo.emplace(UndoInfo{FullPiece{them, static_cast<Piece>(type)}, move.to});

                boards_[them][type].clear(move.to);
                break;
            }
        }

        boards_[side_to_move_][move.type].set(move.to);
        boards_[side_to_move_][move.type].clear(move.from);

        side_to_move_ = them;

        return undo;
    }

    void UndoMove(Move move, const std::optional<UndoInfo>& undo_info) {
        side_to_move_ = side_to_move_ == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
        
        boards_[side_to_move_][move.type].clear(move.to);
        boards_[side_to_move_][move.type].set(move.from);

        if(undo_info) {
            const auto undo = *undo_info;
            const auto captured = undo.captured_piece;

            boards_[captured.color][captured.piece].set(undo.square);
        }
    }

    float Evaluate() const {
        float eval = 0.0f;
        for(int type{ PAWN }; type < KING; ++type) {
            eval += PieceVal(static_cast<Piece>(type)) * boards_[SIDE_WHITE][type].getCount();
            eval -= PieceVal(static_cast<Piece>(type)) * boards_[SIDE_BLACK][type].getCount();
        }

        return side_to_move_ == SIDE_WHITE ? eval : -eval; // return eval relative to side
    }

    float Negamax(int depth) {
        if(depth == 0) return Evaluate();

        float best = -std::numeric_limits<float>::infinity();

        for(auto const move : ValidMoves_()) {
            auto undo = MakeMove(move);
            Print();

            float score = -Negamax(depth - 1);

            UndoMove(move, undo);
            Print();
            
            best = std::max(best, score);
        }

        return best;
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
    std::vector<Move> ValidMoves_() {
        std::vector<Move> moves;
        Bitboard knights = boards_[side_to_move_][KNIGHT];

        Square from;
        while((from = knights.getfstsqr()) != NONE) {
            knights.clear(from);

            Bitboard attacks = MoveTables::kKnightAttacks[from] & ~Occupancy(side_to_move_);

            while(attacks) {
                Square to = attacks.getfstsqr();
                attacks.clear(to);
                moves.push_back({from, to, KNIGHT});
            }
        }
        
        return moves;
    }

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
    // Position board("rnbqkbnr/1pp1pppp/p7/3P4/8/8/PPPP1PPP/RNBQKBNR");
    Position board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    board.Print();

    // std::cout << board.Evaluate() << '\n';
    std::cout << board.Negamax(3);

    // Print8x8(MoveTables::kKingMoves[D4]);
    // Print8x8(Files::A);
}
