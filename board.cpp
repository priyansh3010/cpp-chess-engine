#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
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

void Board::loadFEN(string FEN) {
    // zero everything out first
    memset(pieces, 0, sizeof(pieces));
    memset(occupancy, 0, sizeof(occupancy));

    vector<string> tokens;
    string token;
    istringstream ss(FEN);
    while (getline(ss, token, ' ')) {
        tokens.push_back(token);
    }

    int currSquare = 56;

    // load chess pieces
    for (char c : tokens[0]) {
        if (isdigit(static_cast<unsigned char>(c))) {
            currSquare += (c - '0');
        }
        else if (c == '/') currSquare -= 16;
        else {
            if (c >= 'a' && c <= 'z') {
                if (c == 'r') {
                    pieces[BLACK][ROOK] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'n') {
                    pieces[BLACK][KNIGHT] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'b') {
                    pieces[BLACK][BISHOP] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'q') {
                    pieces[BLACK][QUEEN] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'k') {
                    pieces[BLACK][KING] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'p') {
                    pieces[BLACK][PAWN] ^= (1ULL << currSquare);
                    currSquare++;
                }
            }
            else {
                if (c == 'R') {
                    pieces[WHITE][ROOK] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'N') {
                    pieces[WHITE][KNIGHT] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'B') {
                    pieces[WHITE][BISHOP] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'Q') {
                    pieces[WHITE][QUEEN] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'K') {
                    pieces[WHITE][KING] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'P') {
                    pieces[WHITE][PAWN] ^= (1ULL << currSquare);
                    currSquare++;
                }
            }
        }
    }

    updateOccupancyBoards(*this);

    // set side to move
    sideToMove = tokens[1] == "w" ? WHITE : BLACK;

    // sets castling
    castlingRights = 0b0000;
    for (char c : tokens[2]) {
        if (c == 'Q') castlingRights ^= 0b1000;
        else if (c == 'K') castlingRights ^= 0b0100;
        else if (c == 'q') castlingRights ^= 0b0010;
        else if (c == 'k') castlingRights ^= 0b0001;
    }

    // sets en-passant square
    if (tokens[3] != "-") {
        int file = tokens[3][0] - 'a';
        int rank = tokens[3][1] - '1';
        enPassantSquare = rank * 8 + file;
    }
    else enPassantSquare = -1;

    // set half move clock
    halfMoveClock = stoi(tokens[4]);

    // set full move number
    fullMoveNumber = stoi(tokens[5]);
}

void Board::init(string FEN) {
    loadFEN(FEN);

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
    
    enPassantSquare = -1; // update enPassant
    
    // basic captures and en passant moves
    if (move.promotionPiece == NONE && !move.isCastle){
        pieces[sideToMove][move.pieceType] ^= (1ULL << move.fromSquare);
        pieces[sideToMove][move.pieceType] ^= (1ULL << move.toSquare);
        
        // checks if move was double pawn push
        if (move.pieceType == PAWN && abs(move.fromSquare - move.toSquare) == 16) {
            enPassantSquare = sideToMove == WHITE ? move.toSquare - 8 : move.toSquare + 8;
        }
        // special conditions if move was enpassant
        else if (move.capturedPiece == PAWN && move.isEnPassant) {
            if (sideToMove == WHITE) {
                pieces[BLACK][PAWN] ^= (1ULL << (move.toSquare - 8));
            }
            if (sideToMove == BLACK) {
                pieces[WHITE][PAWN] ^= (1ULL << (move.toSquare + 8));
            }
            halfMoveClock = 0;
        }
        // checks if normal capture took place
        else if (move.capturedPiece != NONE) {
            pieces[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece] ^= (1ULL << move.toSquare);
            halfMoveClock = 0;

            // update castling rights if captured piece was rook
            if (move.capturedPiece == ROOK) {
                if (sideToMove == BLACK) {
                    if (move.toSquare == 0) {
                        if (castlingRights & 0b0100) castlingRights ^= 0b0100;
                    }
                    else if (move.toSquare == 7) {
                        if (castlingRights & 0b1000) castlingRights ^= 0b1000;
                    }
                }
                else {
                    if (move.toSquare == 56) {
                        if (castlingRights & 0b0001) castlingRights ^= 0b0001;
                    }
                    else if (move.toSquare == 63) {
                        if (castlingRights & 0b0010) castlingRights ^= 0b0010;
                    }
                }
            }
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
            halfMoveClock = 0;

            // update castling rights if captured piece was rook
            if (move.capturedPiece == ROOK) {
                if (sideToMove == BLACK) {
                    if (move.toSquare == 0) {
                        if (castlingRights & 0b0100) castlingRights ^= 0b0100;
                    }
                    else if (move.toSquare == 7) {
                        if (castlingRights & 0b1000) castlingRights ^= 0b1000;
                    }
                }
                else {
                    if (move.toSquare == 56) {
                        if (castlingRights & 0b0001) castlingRights ^= 0b0001;
                    }
                    else if (move.toSquare == 63) {
                        if (castlingRights & 0b0010) castlingRights ^= 0b0010;
                    }
                }
            }
        }
    }
    // castling moves
    else if (move.isCastle) {
        if (move.toSquare > move.fromSquare) {
            // move rook to correct spot
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare + 3);
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare + 1);
        }
        else {
            // move rook to correct spot
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare - 4);
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare - 1);
        }
        if (sideToMove == WHITE) castlingRights ^= 0b1100;
        if (sideToMove == BLACK) castlingRights ^= 0b0011;
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
    if (move.capturedPiece != NONE && !move.isEnPassant) {
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

bool Board::isKingInCheck(Color kingColor) {
    int kingIndex = getLSB(pieces[kingColor][KING]);
    return isSquareAttacked(kingColor == WHITE ? BLACK : WHITE, kingIndex);
} 

bool Board::isSquareAttacked(Color attackingPlayer, int toSquare) {
    Color currColor = attackingPlayer == WHITE ? BLACK : WHITE;
    // Knights
    if (MoveGen::knightAttacks[toSquare] & pieces[attackingPlayer][KNIGHT]) return true;
    // Kings
    if (MoveGen::kingAttacks[toSquare] & pieces[attackingPlayer][KING]) return true;
    // Pawns
    U64 sqMask = 1ULL << toSquare;
    if (attackingPlayer == WHITE) {
        U64 pawnAttacks = ((sqMask >> 7) & ~MASK_A_FILE) | ((sqMask >> 9) & ~MASK_H_FILE);
        if (pawnAttacks & pieces[attackingPlayer][PAWN]) return true;
    } else {
        U64 pawnAttacks = ((sqMask << 7) & ~MASK_H_FILE) | ((sqMask << 9) & ~MASK_A_FILE);
        if (pawnAttacks & pieces[attackingPlayer][PAWN]) return true;
    }
    // Bishops + Queens
    if (MoveGen::getRays(toSquare, occupancy[currColor], occupancy[attackingPlayer], DIAG_DIRS) &
       (pieces[attackingPlayer][BISHOP] | pieces[attackingPlayer][QUEEN])) return true;
    // Rooks + Queens
    if (MoveGen::getRays(toSquare, occupancy[currColor], occupancy[attackingPlayer], STRAIGHT_DIRS) &
       (pieces[attackingPlayer][ROOK] | pieces[attackingPlayer][QUEEN])) return true;
    return false;
}