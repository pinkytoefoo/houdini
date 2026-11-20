#include <ios>
#include <iostream>
#include <cstdint>
#include <limits>
#include <string>
#include <cassert>

#include "position.hpp"

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
    // Position board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    // Position board("8/8/3B4/R1b1k3/8/2K1Q3/3R1pp1/8");
    Position board("8/8/3B4/R1b2k2/8/2K1Q3/3R1pp1/8");
    std::cout << board << '\n';

    // std::cout << board.Evaluate() << '\n';
    int initialAlpha = -std::numeric_limits<int>::max();
    int initialBeta  =  std::numeric_limits<int>::max();
    int searchDepth  =  4;
    std::cout << board.Negamax(searchDepth, initialAlpha, initialBeta);

    // Print8x8(MoveTables::kKingMoves[D4]);
    // Print8x8(Files::A);
}

