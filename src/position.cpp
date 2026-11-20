#include <cstdint>
#include <limits>

#include "position.hpp"

Position::Position(const std::string& fen) {
    LoadPositionFromFen(fen);
}

void Position::LoadPositionFromFen(const std::string& fen) {
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
            PutPiece_(static_cast<Square>(rank * 8 + file), cp.color, cp.piece);
            ++file;
        }
    }
}

[[nodiscard]] bool Position::TryMove(Move move) {
    if(!Piece_(side_to_move_, move.type)) {
        return false;
    }

    if(move.type == kKnight) {
        Bitboard attacks = MoveTables::kKnightAttacks[move.from];
        Bitboard valid = attacks & ~occupancies_[side_to_move_];
        // todo: add king check logic
    }

    MakeMove(move);
    return true;
}

void Position::MakeMove(Move move) {
    std::optional<UndoInfo> undo{std::nullopt};
    Side us = side_to_move_;
    Side them = ~side_to_move_;
    
    for(uint8_t type{}; type < kKing; ++type) {
        if(Piece_(them, static_cast<Piece>(type)).Get(move.to)) {
            undo.emplace(FullPiece{them, static_cast<Piece>(type)}, move.to);
            ClearPiece_(move.to, them, static_cast<Piece>(type));
            break;
        }
    }
    
    PutPiece_(move.to, side_to_move_, move.type);
    ClearPiece_(move.from, side_to_move_, move.type);

    moves_info_.emplace(move, std::move(undo));
    side_to_move_ = them;
}


void Position::UndoMove() {
    side_to_move_ = ~side_to_move_;
    
    auto [move, undo_info] = moves_info_.top();
    
    ClearPiece_(move.to, side_to_move_, move.type);
    PutPiece_(move.from, side_to_move_, move.type);
    
    if(undo_info) {
        const auto undo = *undo_info;
        const auto captured = undo.captured_piece;
        
        PutPiece_(undo.square, captured.color, captured.piece);
    }

    moves_info_.pop();
}

int Position::Evaluate() const {
    int eval = 0;
    for(int type{ kPawn }; type < kKing; ++type) {
        eval += kPieceValues[static_cast<Piece>(type)] * Piece_(kWhite, static_cast<Piece>(type)).GetCount();
        eval -= kPieceValues[static_cast<Piece>(type)] * Piece_(kBlack, static_cast<Piece>(type)).GetCount();
    }

    return side_to_move_ == kWhite ? eval : -eval; // return eval relative to side
}

int Position::Negamax(int depth, int alpha, int beta) {
    if(depth == 0) {
        return Evaluate();
    }

    int best = -std::numeric_limits<int>::max();

    auto moves = ValidMoves_();

    for(auto const move : moves) {
        MakeMove(move);
        std::cout << *this << '\n';

        int score = -Negamax(depth - 1, -beta, -alpha);

        UndoMove();
        std::cout << *this << '\n';
       
        if(score >= beta) {
            return beta;
        }

        if(score > best) {
            best = score;
            alpha = std::max(alpha, score);
        }
    }

    return best;
}

std::ostream& operator<<(std::ostream& os, const Position& pos) {
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

            for (u16 side{}; side < kEmptySide; ++side) {
                for(u16 piece{}; piece < kEmptyPiece; ++piece)
                if(pos.Piece_(static_cast<Side>(side), static_cast<Piece>(piece)).Get(square)) {
                    piece_rep = tokens[side][piece];
                    break;
                }
            }

            std::cout << piece_rep << " | ";
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "    a   b   c   d   e   f   g   h\n\n";

    return os;
}

