#include <iostream>
#include "board.h"
using namespace std;

void Board::init() {
    // Setting white pieces
    pieces[WHITE][ROOK] = (1ULL << a1) | (1ULL << h1);
    pieces[WHITE][KNIGHT] = (1ULL << b1) | (1ULL << g1);
    pieces[WHITE][BISHOP] = (1ULL << c1) | (1ULL << f1);
    pieces[WHITE][KING] = (1ULL << e1);
    pieces[WHITE][QUEEN] = (1ULL << d1);
    pieces[WHITE][PAWN] = (1ULL << a2) | (1ULL << b2) | (1ULL << c2) | (1ULL << d2)
                        | (1ULL << e2) | (1ULL << f2) | (1ULL << g2) | (1ULL << h2);
                     
    // Setting black pieces
    pieces[BLACK][ROOK] = (1ULL << a8) | (1ULL << h8);
    pieces[BLACK][KNIGHT] = (1ULL << b8) | (1ULL << g8);
    pieces[BLACK][BISHOP] = (1ULL << c8) | (1ULL << f8);
    pieces[BLACK][KING] = (1ULL << e8);
    pieces[BLACK][QUEEN] = (1ULL << d8);
    pieces[BLACK][PAWN] = (1ULL << a7) | (1ULL << b7) | (1ULL << c7) | (1ULL << d7)
                        | (1ULL << e7) | (1ULL << f7) | (1ULL << g7) | (1ULL << h7);

    // Setting occupancy bttboards
    occupancy[WHITE] = pieces[WHITE][ROOK] | pieces[WHITE][KNIGHT]
                     | pieces[WHITE][BISHOP] | pieces[WHITE][KING]
                     | pieces[WHITE][QUEEN] | pieces[WHITE][PAWN];

    occupancy[BLACK] = pieces[BLACK][ROOK] | pieces[BLACK][KNIGHT]
                    | pieces[BLACK][BISHOP] | pieces[BLACK][KING]
                    | pieces[BLACK][QUEEN] | pieces[BLACK][PAWN];
    
    occupancy[ALL] = occupancy[BLACK] | occupancy[WHITE];

    // initialising other variables
    sideToMove = WHITE;
    castlingRights = 0b1111; // first 2 bits for white castling, last 2 for black castling
    enPassantSquare = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
}

void Board::printBoard() {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file <= 7; file++) {
            int sq = rank * 8 + file;
            U64 mask = 1ULL << sq;

            char currPiece = '.';

            if (pieces[WHITE][ROOK] & mask) currPiece = 'R';
            else if (pieces[WHITE][KNIGHT] & mask) currPiece = 'N';
            else if (pieces[WHITE][BISHOP] & mask) currPiece = 'B';
            else if (pieces[WHITE][QUEEN] & mask) currPiece = 'Q';
            else if (pieces[WHITE][KING] & mask) currPiece = 'K';
            else if (pieces[WHITE][PAWN] & mask) currPiece = 'P';
            
            else if (pieces[BLACK][ROOK] & mask) currPiece = 'r';
            else if (pieces[BLACK][KNIGHT] & mask) currPiece = 'n';
            else if (pieces[BLACK][BISHOP] & mask) currPiece = 'b';
            else if (pieces[BLACK][QUEEN] & mask) currPiece = 'q';
            else if (pieces[BLACK][KING] & mask) currPiece = 'k';
            else if (pieces[BLACK][PAWN] & mask) currPiece = 'p';

            cout << currPiece;
        }
        cout << endl;
    }
}

Piece Board::getPieceAt(Color color, int square) const {
    U64 mask = 1ULL << square;
    for (int pt = KING; pt <= PAWN; pt++) {
        if (pieces[color][pt] & mask)
            return static_cast<Piece>(pt);
    }
    return NONE;
}