#pragma once

#include <cstdint>
#include <bit>

#include "types.hpp"

// inspired by follychess engine
class [[nodiscard]] Bitboard {
public:
    
    Bitboard() = default;

    constexpr explicit Bitboard(std::uint64_t bb)
        : data_{ bb }
    {
    }
    
    constexpr explicit Bitboard(Square square)
        : data_{ 1ull << square }
    {
    }

    [[nodiscard]] constexpr bool Get(Square square) const {
        return (data_ & (1ull << square)) != 0;
    }

    constexpr void Set(Square square) {
        data_ |= (1ull << square);
    }
    
    constexpr void Clear(Square square) {
        data_ &= ~(1ull << square);
    }

    // get the first square
    constexpr Square FirstSquare() const {
        return data_ == 0
            ? NONE
            : static_cast<Square>(std::countr_zero(data_));
    }

    constexpr Bitboard operator<<(int bits) const {
        return Bitboard(data_ << bits);
    }

    constexpr Bitboard operator>>(int bits) const {
        return Bitboard(data_ >> bits);
    }

    constexpr Bitboard operator~() const {
        return Bitboard(~data_);
    }

    constexpr Bitboard operator&(Bitboard other) const {
        return Bitboard(data_ & other.data_);
    }

    constexpr Bitboard operator|(Bitboard other) const {
        return Bitboard(data_ | other.data_);
    }

    constexpr Bitboard& operator&=(Bitboard other) {
        data_ |= other.data_;
        return *this;
    }

    constexpr Bitboard& operator|=(Bitboard other) {
        data_ |= other.data_;
        return *this;
    }

    constexpr operator bool() const {
        return data_ != 0;
    }

    [[nodiscard]] constexpr int GetCount() const { return std::popcount(data_); }
    
    template<Direction direction>
    constexpr Bitboard shift() const;

    constexpr bool operator==(const Bitboard& other) const = default;
    constexpr auto operator<=>(Bitboard other);

private:
    std::uint64_t data_{};
};

namespace Files {
    inline constexpr Bitboard A{ 0x0101010101010101ULL };
    inline constexpr Bitboard AB = A << 1;
    inline constexpr Bitboard H{ 0x8080808080808080ULL };
    inline constexpr Bitboard GH{ H >> 1 };
}

template<Direction Direction>
constexpr Bitboard Bitboard::shift() const {
    if constexpr(Direction == North)     { return *this << 8; }

    if constexpr(Direction == NorthEast) { return *this << 7 & ~Files::A; }

    if constexpr(Direction == East)      { return *this >> 1 & ~Files::A; }

    if constexpr(Direction == SouthEast) { return *this >> 9 & ~Files::A; }

    if constexpr(Direction == South)     { return *this >> 8; }

    if constexpr(Direction == SouthWest) { return *this >> 7 & ~Files::H; }

    if constexpr(Direction == West)      { return *this << 1 & ~Files::H; }

    if constexpr(Direction == NorthWest) { return *this << 9 & ~Files::H; }

    return Bitboard{};
}

