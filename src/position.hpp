#pragma once

#include <string>
#include <cassert>
#include <optional>
#include <iostream>
#include <stack>
#include <vector>

#include "bitboard.hpp"
#include "move_tables.hpp"
#include "types.hpp"

class Position
{
public:
    Position(const std::string& fen);

    void LoadPositionFromFen(const std::string& fen);

    [[nodiscard]] bool TryMove(Move move);

    void MakeMove(Move move);

    void UndoMove();

    int Evaluate() const;

    int Negamax(int depth, int alpha, int beta);

    friend std::ostream& operator<<(std::ostream& os, const Position& pos);

    // todo: check if there is really a knight on from square
    Bitboard KnightPsuedoMoves(Square from, Side side) {
        return MoveTables::kKnightAttacks[from] & ~occupancies_[side];
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
                if (Piece_(static_cast<Side>(color), static_cast<Piece>(piece)).Get(square)) {
                    return tokens[color][piece];
                }
            }
        }
        return '.';
    }

private:
    std::vector<Move> ValidMoves_() {
        std::vector<Move> moves;
        Bitboard knights = Piece_(side_to_move_, kKnight);

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

        return moves;
    }

    __attribute__((always_inline))
    void PutPiece_(Square square, Side side, Piece piece) {
        occupancies_[side].Set(square);
        pieces_[piece].Set(square);
    }

    __attribute__((always_inline))
    void ClearPiece_(Square square, Side side, Piece piece) {
        occupancies_[side].Clear(square);
        pieces_[piece].Clear(square);
    }

    Bitboard Piece_(Side side, Piece piece) const {
        return occupancies_[side] & pieces_[piece];
    }

    static constexpr int kPieceValues[] = {100, 300, 300, 500, 900, 0};

    std::array<Bitboard, kEmptyPiece> pieces_;
    std::array<Bitboard, 3> occupancies_;
    Side side_to_move_{kWhite};
    std::optional<Square> en_passant_sq_;

    std::stack<std::pair<Move, std::optional<UndoInfo>>> moves_info_;
};

