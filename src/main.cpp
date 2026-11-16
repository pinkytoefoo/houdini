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
    kPawn,
    kKnight,
    kBishop,
    kRook,
    kQueen,
    kKing,
    kEmptyPiece
};

// todo: add square weights and game stage (early, mid, late game weights)
int PieceVal(Piece type) {
    switch(type) {
        case kPawn: return 1;
        case kKnight: case kBishop: return 3;
        case kRook: return 5;
        case kQueen: return 9;
        default:
            throw std::logic_error{"piece values are only assigned to valued pieces"};
    }
}

enum Side : std::uint16_t {
    kWhite,
    kBlack,
    kEmptySide,
};

constexpr Side operator~(Side side) {
    if(side == kEmptySide) {
        throw std::logic_error{"side is not white or black"};
    }
    return side == kWhite ? kBlack : kWhite;
}

struct FullPiece {
    Side color;
    Piece piece;
};

constexpr FullPiece CharToPiece(char ch) {
    switch (ch) {
        case 'P': return {kWhite, kPawn};
        case 'N': return {kWhite, kKnight};
        case 'B': return {kWhite, kBishop};
        case 'R': return {kWhite, kRook};
        case 'Q': return {kWhite, kQueen};
        case 'K': return {kWhite, kKing};

        case 'p': return {kBlack, kPawn};
        case 'n': return {kBlack, kKnight};
        case 'b': return {kBlack, kBishop};
        case 'r': return {kBlack, kRook};
        case 'q': return {kBlack, kQueen};
        case 'k': return {kBlack, kKing};
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
    Position(const std::string& fen) {
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
        if(!boards_[side_to_move_][type].Get(move.from)) {
            return false;
        }

        if(type == kKnight) {
            Bitboard attacks = MoveTables::kKnightAttacks[move.from];
            Bitboard valid = attacks & ~Occupancy(side_to_move_);
            // todo: add king check logic
        }

        MakeMove(move);
        return true;
    }

    std::optional<UndoInfo> MakeMove(Move move) {
        std::optional<UndoInfo> undo{std::nullopt};
        Side us = side_to_move_;
        Side them = ~side_to_move_;
        
        for(int type{}; type < kQueen; ++type) {
            if(boards_[them][static_cast<Piece>(type)].Get(move.to)) {
                undo.emplace(UndoInfo{FullPiece{them, static_cast<Piece>(type)}, move.to});

                boards_[them][type].Clear(move.to);
                break;
            }
        }

        boards_[side_to_move_][move.type].Set(move.to);
        boards_[side_to_move_][move.type].Clear(move.from);

        side_to_move_ = them;

        return undo;
    }

    void UndoMove(Move move, const std::optional<UndoInfo>& undo_info) {
        side_to_move_ = ~side_to_move_;
        
        boards_[side_to_move_][move.type].Clear(move.to);
        boards_[side_to_move_][move.type].Set(move.from);

        if(undo_info) {
            const auto undo = *undo_info;
            const auto captured = undo.captured_piece;

            boards_[captured.color][captured.piece].Set(undo.square);
        }
    }

    float Evaluate() const {
        float eval = 0.0f;
        for(int type{ kPawn }; type < kKing; ++type) {
            eval += PieceVal(static_cast<Piece>(type)) * boards_[kWhite][type].GetCount();
            eval -= PieceVal(static_cast<Piece>(type)) * boards_[kBlack][type].GetCount();
        }

        return side_to_move_ == kWhite ? eval : -eval; // return eval relative to side
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

                for (u16 color{}; color < kEmptySide; ++color) {
                    for(u16 piece{}; piece < kEmptyPiece; ++piece)
                    if(boards_[color][piece].Get(square)) {
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
        Bitboard result{};

        for(short piece{}; piece < kEmptyPiece; ++piece) {
            result |= boards_[side][piece];
        }

        return result;
    }

    [[nodiscard]] Bitboard Occupancy() const {
        return Occupancy(kWhite) | Occupancy(kBlack);
    }

    // todo: check if there is really a knight on from square
    Bitboard KnightPsuedoMoves(Square from, Side side) {
        return MoveTables::kKnightAttacks[from] & ~Occupancy(side);
    }

    Bitboard PawnPsuedoMoves(Square from, Side side) {
        //
        // return 
    }
    
    char GetPieceAt(Square square) const {
        static constexpr char tokens[][6] = {
            {'P', 'N', 'B', 'R', 'Q', 'K'},
            {'p', 'n', 'b', 'r', 'q', 'k'}
        };

        for (u16 color{}; color < kEmptySide; ++color) {
            for (u16 piece{}; piece < kEmptyPiece; ++piece) {
                if (boards_[color][piece].Get(square)) {
                    return tokens[color][piece];
                }
            }
        }
        return '.';
    }

private:
    std::vector<Move> ValidMoves_() {
        std::vector<Move> moves;
        Bitboard knights = boards_[side_to_move_][kKnight];

        Square from;
        while((from = knights.FirstSquare()) != NONE) {
            knights.Clear(from);

            Bitboard attacks = KnightPsuedoMoves(from, side_to_move_);

            while(attacks) {
                Square to = attacks.FirstSquare();
                attacks.Clear(to);
                moves.push_back({from, to, kKnight});
            }
        }
        
        Bitboard pawns = boards_[side_to_move_][kPawn];
        while((from = pawns.FirstSquare()) != NONE) {
            pawns.Clear(from);

            Bitboard attacks = KnightPsuedoMoves(from, side_to_move_);

            while(attacks) {
                Square to = attacks.FirstSquare();
                attacks.Clear(to);
                moves.push_back({from, to, kPawn});
            }
        }
        
        return moves;
    }

    Bitboard boards_[kEmptySide][kEmptyPiece]{{}};
    Side side_to_move_{kWhite};
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

            for (u16 side{}; side < kEmptySide; ++side) {
                for(u16 piece{}; piece < kEmptyPiece; ++piece)
                if (bb.Get(square)) {
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
