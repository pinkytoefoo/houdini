#pragma once

#include <cstdint>
#include <stdexcept>

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

enum Direction : u8 {
    North, NorthEast, East, SouthEast,
    South, SouthWest, West, NorthWest,
};

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

enum Piece : std::uint16_t {
    kPawn,
    kKnight,
    kBishop,
    kRook,
    kQueen,
    kKing,
    kEmptyPiece
};

enum Side : std::uint16_t {
    kWhite,
    kBlack,
    kEmptySide,
    kSideCount,
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


