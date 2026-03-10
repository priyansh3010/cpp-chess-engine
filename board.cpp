#include <iostream>
#include <cmath>
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "moveinfo.h"
using namespace std;

namespace {
    void updateOccupancyBoards(Board& board) {
        board.occupancy[WHITE] = board.pieces[WHITE][ROOK] | board.pieces[WHITE][KNIGHT]
                                | board.pieces[WHITE][BISHOP] | board.pieces[WHITE][KING]
                                | board.pieces[WHITE][QUEEN] | board.pieces[WHITE][PAWN];

        board.occupancy[BLACK] = board.pieces[BLACK][ROOK] | board.pieces[BLACK][KNIGHT]
                                | board.pieces[BLACK][BISHOP] | board.pieces[BLACK][KING]
                                | board.pieces[BLACK][QUEEN] | board.pieces[BLACK][PAWN];

        board.occupancy[ALL] = board.occupancy[WHITE] | board.occupancy[BLACK];
    }
}

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

    // pre-compute knight moves
    MoveGen::init();
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

MoveInfo Board::makeMove(Move move) {
    MoveInfo moveInfo(move, enPassantSquare, castlingRights, halfMoveClock, fullMoveNumber, sideToMove);
    // basic captures and en passant moves
    if (move.promotionPiece == NONE && move.isCastle){
        pieces[sideToMove][move.pieceType] ^= (1ULL << move.fromSquare);
        pieces[sideToMove][move.pieceType] ^= (1ULL << move.toSquare);

        // checks if move was double pawn push
        if (move.pieceType == PAWN && abs(move.fromSquare - move.toSquare) == 16) {
            enPassantSquare = sideToMove == WHITE ? move.toSquare - 8 : move.toSquare + 8;
        }
        // special conditions if move was enpassant
        else if (move.capturedPiece == PAWN && move.isEnPassant) {
            if (sideToMove == WHITE) {
                pieces[BLACK][PAWN] ^= (1ULL - (move.toSquare - 8));
                halfMoveClock = 0;
                enPassantSquare = -1;
            }
            if (sideToMove == BLACK) {
                pieces[WHITE][PAWN] ^= (1ULL - (move.toSquare + 8));
                halfMoveClock = 0;
            }
        }
        // checks if normal capture took place
        else if (move.capturedPiece != NONE) {
            pieces[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece] ^= (1ULL << move.toSquare);
            halfMoveClock = 0;
        }

        // update castling rights if necessary
        if (move.pieceType == ROOK) {
            if (sideToMove == WHITE) {
                if (move.fromSquare == 0) {
                    if (castlingRights & 0b0100) castlingRights ^= 0b0100;
                }
                else if (move.fromSquare == 7) {
                    if (castlingRights & 0b1000) castlingRights ^= 0b1000;
                }
            }
            else {
                if (move.fromSquare == 56) {
                    if (castlingRights & 0b0001) castlingRights ^= 0b0001;
                }
                else if (move.fromSquare == 63) {
                    if (castlingRights & 0b0010) castlingRights ^= 0b0010;
                }
            }
        }
        else if (move.pieceType == KING) {
            if (sideToMove == WHITE) castlingRights &= 0b0011;
            else castlingRights &= 0b1100;
        }
    }
    // promotion move
    else if (move.promotionPiece != NONE) {
        pieces[sideToMove][PAWN] ^= (1ULL << move.fromSquare); // removes pawn from bitboard
        pieces[sideToMove][move.promotionPiece] ^= (1ULL << move.toSquare); // adds promoted pieces to appropriate bitboard

        // removes captured piece from appropriate bitboard
        if (move.capturedPiece != NONE) {
            pieces[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece] ^= (1ULL << move.toSquare);
        }
    }
    // castling moves
    else if (move.isCastle) {
        if (move.toSquare > move.fromSquare) {
            // move rook to correct spot
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare + 3);
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare + 1);
            if (sideToMove == WHITE) castlingRights ^= 0b1000;
            if (sideToMove == BLACK) castlingRights ^= 0b0010;
        }
        else {
            // move rook to correct spot
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare - 4);
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare - 1);
            if (sideToMove == WHITE) castlingRights ^= 0b0100;
            if (sideToMove == BLACK) castlingRights ^= 0b0001;
        }
        // move king to correct spot
        pieces[sideToMove][KING] ^= (1ULL << move.fromSquare);
        pieces[sideToMove][KING] ^= (1ULL << move.toSquare);
    }

    // halfMoveClock update
    if (move.pieceType == PAWN || move.capturedPiece != NONE) halfMoveClock = 0;
    else halfMoveClock++;

    // fullMove update
    if (sideToMove == BLACK) fullMoveNumber++;

    sideToMove = sideToMove == WHITE ? BLACK : WHITE;

    updateOccupancyBoards(*this);
    return moveInfo;
}

void Board::unMakeMove(MoveInfo moveInfo) {
    Move move = moveInfo.move;

    // move piece back to previous standing place
    pieces[moveInfo.prevColor][move.pieceType] ^= (1ULL << move.toSquare);
    pieces[moveInfo.prevColor][move.pieceType] ^= (1ULL << move.fromSquare);

    // check if castle
    if (move.isCastle) {
        if (move.toSquare > move.fromSquare) {
            // move rook to back to original spot
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare + 1);
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare + 3);
        }
        else {
            // move rook to back to original spot
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare - 1);
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare - 4);
        }
    }
    // check if promotion
    else if (move.promotionPiece != NONE) {
        pieces[moveInfo.prevColor][PAWN] ^= (1ULL << move.toSquare); // undo the accidental creation of pawn above in the function
        pieces[moveInfo.prevColor][move.promotionPiece] ^= (1ULL << move.toSquare); // remove promoted piece
    }
    else if (move.isEnPassant) {
        // if captured pawn was white
        if (sideToMove == WHITE) {
            pieces[WHITE][PAWN] ^= (1ULL << (moveInfo.prevEnPassantSquare + 8));
        }
        // if captured pawn was black
        else {
            pieces[BLACK][PAWN] ^= (1ULL << (moveInfo.prevEnPassantSquare - 8));
        }
    }
    // all normal captures
    else if (move.capturedPiece != NONE) {
        pieces[sideToMove][move.capturedPiece] ^= (1ULL << move.toSquare);
    }

    // reset all variables to previous state
    sideToMove = moveInfo.prevColor;
    enPassantSquare = moveInfo.prevEnPassantSquare;
    castlingRights = moveInfo.prevCastling;
    halfMoveClock = moveInfo.prevHalfMoveClock;
    fullMoveNumber = moveInfo.prevFullMoveNumber;

    // update all occupancy boards
    updateOccupancyBoards(*this);
}

bool Board::isKingInCheck() {
    int kingIndex = getLSB(pieces[sideToMove == WHITE ? BLACK : WHITE][KING]);
    vector<Move> allMoves = MoveGen::generateAllMoves(*this);

    for (Move move : allMoves) {
        if (move.toSquare == kingIndex) return true;
    }

    return false;
} 