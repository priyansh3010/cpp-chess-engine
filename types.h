#pragma once
#include <cstdint>
#include <intrin.h>
#include <array>

typedef uint64_t U64;

enum Piece { KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN, NONE };
enum Color { WHITE, BLACK, ALL };

enum Square {
    // comments represnet bitboard squares
    a1, b1, c1, d1, e1, f1, g1, h1, // h8 g8 f8 e8 d8 c8 b8 a8 || 63 62 61 60 59 58 57 56
    a2, b2, c2, d2, e2, f2, g2, h2, // h7 g7 f7 e7 d7 c7 b7 a7 || 55 54 53 52 51 50 49 48
    a3, b3, c3, d3, e3, f3, g3, h3, // h6 g6 f6 e6 d6 c6 b6 a6 || 47 46 45 44 43 42 41 40
    a4, b4, c4, d4, e4, f4, g4, h4, // h5 g5 f5 e5 d5 c5 b5 a5 || 39 38 37 36 35 34 33 32
    a5, b5, c5, d5, e5, f5, g5, h5, // h4 g4 f4 e4 d4 c4 b4 a4 || 31 30 29 28 27 26 25 24
    a6, b6, c6, d6, e6, f6, g6, h6, // h3 g3 f3 e3 d3 c3 b3 a3 || 23 22 21 20 19 18 17 16
    a7, b7, c7, d7, e7, f7, g7, h7, // h2 g2 f2 e2 d2 c2 b2 a2 || 15 14 13 12 11 10 09 08
    a8, b8, c8, d8, e8, f8, g8, h8  // h1 g1 f1 e1 d1 c1 b1 a1 || 07 06 05 04 03 02 01 00
};

const int NUM_SQUARES = 64;
const int NUM_UNIQUE_PIECES = 6;

inline int getLSB(U64 bitboard) {
    unsigned long index;
    _BitScanForward64(&index, bitboard);
    return (int)index;
}

const U64 MASK_A_FILE = 0x0101010101010101ULL; // A file mask | 00000001
const U64 MASK_H_FILE = 0x8080808080808080ULL; // H file mask | 10000000
const U64 MASK_A_B_FILE = 0x0303030303030303ULL; // A and B file mask | 00000011
const U64 MASK_G_H_FILE = 0xC0C0C0C0C0C0C0C0ULL; // G and H file mask | 11000000

// defines direction struct for sliding piece movegen
struct Direction { int shift; U64 noWrap; };

inline const Direction DIAG_DIRS[4] = {
    { +9, ~MASK_A_FILE }, // north west
    { +7, ~MASK_H_FILE }, // north east
    { -9, ~MASK_H_FILE }, // south east
    { -7, ~MASK_A_FILE }  // south west
};
    
inline const Direction STRAIGHT_DIRS[4] = {
    { +8, ~0ULL },        // north
    { -8, ~0ULL },        // south 
    { +1, ~MASK_A_FILE }, // west
    { -1, ~MASK_H_FILE }  // east
};