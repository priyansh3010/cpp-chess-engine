#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "movegen.h"
#include "move.h"
#include "types.h"
using namespace std;

namespace {
    U64 knightAttacks[64];
    // pre-compute knight moves
    // direction is based on the orientation of bitboards defined in comments in types.h
    void preComputeKnightMoves() {
        for (int i = 0; i < 64; i++) {
            U64 currIndex = 1ULL << i;
            U64 northWestWest = (currIndex << 6) & ~MASK_G_H_FILE;
            U64 northNorthWest = (currIndex << 15) & ~MASK_H_FILE;
            U64 northNorthEast = (currIndex << 17) & ~MASK_A_FILE;
            U64 northEastEast = (currIndex << 10) & ~MASK_A_B_FILE;
            U64 southEastEast = (currIndex >> 6) & ~MASK_A_B_FILE;
            U64 southSouthEast = (currIndex >> 15) & ~MASK_A_FILE;
            U64 southSouthWest = (currIndex >> 17) & ~MASK_H_FILE;
            U64 southWestWest = (currIndex >> 10) & ~MASK_G_H_FILE;
            
            U64 currMoveMap = northWestWest | northNorthWest | northNorthEast | northEastEast
                            | southWestWest | southSouthWest | southSouthEast | southEastEast;
            
            knightAttacks[i] = currMoveMap;
        }
    }
    // Pawn move functions
    vector<Move> wPawnSinglePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank7Mask = 0x00FF000000000000ULL; // Will highlight all pawns on 7th rank

        U64 singlePush = ((board.pieces[WHITE][PAWN] & ~rank7Mask) << 8) & emptySquares; // all squares white pawns can move to
        
        U64 rank7Pawns = board.pieces[WHITE][PAWN] & rank7Mask; // all squares 7th rank white pawns can move to
        U64 promoPawns = (rank7Pawns << 8) & emptySquares;
        
        vector<Move> moveList;
        
        // Find all 1 bits to generate single push moves
        while (singlePush) {
            int toSquare = getLSB(singlePush);
            int fromSquare = toSquare - 8;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list
            
            singlePush &= singlePush - 1; // move onto next least significant bit
        }
        
        // all promo moves possible
        while (promoPawns) {
            int toSquare = getLSB(promoPawns);
            int fromSquare = toSquare - 8;
            
            // All 4 promotions
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, ROOK)); 
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, BISHOP));             
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, KNIGHT)); 
            
            promoPawns &= promoPawns - 1;
        }
        
        return moveList;
    }
    vector<Move> wPawnDoublePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank2Pawns = board.pieces[WHITE][PAWN] & 0x000000000000FF00ULL;
        U64 singlePush = (rank2Pawns << 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush << 8) & emptySquares;       // destination must also be empty

        vector<Move> moveList;

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare - 16;
            Move move(fromSquare, toSquare, PAWN, NONE, NONE, false, true);
            moveList.push_back(move); // add to move list

            doublePush &= doublePush - 1; // move onto next least significant bit
        }

        return moveList;
    }
    vector<Move> wPawnCapture(const Board& board) {
        U64 mask7Rank = 0x00FF000000000000ULL; // mask 7th rank
        U64 leftCaptures  = ((board.pieces[WHITE][PAWN] & ~mask7Rank) << 7) & board.occupancy[BLACK] & ~MASK_H_FILE;
        U64 rightCaptures = ((board.pieces[WHITE][PAWN] & ~mask7Rank) << 9) & board.occupancy[BLACK] & ~MASK_A_FILE;
        U64 rank7Pawns = board.pieces[WHITE][PAWN] & mask7Rank;
        U64 leftPromoCaptures = (rank7Pawns << 7) & board.occupancy[BLACK] & ~MASK_H_FILE;
        U64 rightPromoCaptures = (rank7Pawns << 9) & board.occupancy[BLACK] & ~MASK_A_FILE;

        vector<Move> moveList;

        // Check left captures
        while (leftCaptures) {
            int toSquare = getLSB(leftCaptures);
            int fromSquare = toSquare - 7;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare - 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
        }

        // Check left promotion captures
        while (leftPromoCaptures) {
            int toSquare = getLSB(leftPromoCaptures);
            int fromSquare = toSquare - 7;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);

            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));
            
            leftPromoCaptures &= leftPromoCaptures - 1; // move onto next least significant bit
        }
        
        // Check right promotion captures
        while (rightPromoCaptures) {
            int toSquare = getLSB(rightPromoCaptures);
            int fromSquare = toSquare - 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);

            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));

            rightPromoCaptures &= rightPromoCaptures - 1; // move onto next least significant bit
        }

        // check for en passant moves
        if (board.enPassantSquare != -1) {   
            if ((board.pieces[WHITE][PAWN] & ~MASK_H_FILE) << 7 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare - 7, board.enPassantSquare, PAWN, PAWN);
                moveList.push_back(move);
            }
            
            if ((board.pieces[WHITE][PAWN] & ~MASK_A_FILE) << 9 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare - 9, board.enPassantSquare, PAWN, PAWN);
                moveList.push_back(move);
            }
        }

        return moveList;
    }
    vector<Move> bPawnSinglePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank2Mask = 0x000000000000FF00ULL; // Will highlight all pawns below 2nd rank

        U64 singlePush = ((board.pieces[BLACK][PAWN] & ~rank2Mask) << 8) & emptySquares; // all squares white pawns can move to
        
        U64 rank2Pawns = board.pieces[BLACK][PAWN] & rank2Mask; // all squares 2nd rank white pawns can move to
        U64 promoPawns = (rank2Pawns >> 8) & emptySquares;

        vector<Move> moveList;

        // Find all 1 bits to generate single push moves
        while (singlePush) {
            int toSquare = getLSB(singlePush);
            int fromSquare = toSquare + 8;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            singlePush &= singlePush - 1; // move onto next least significant bit
        }

        while (promoPawns) {
            int toSquare = getLSB(promoPawns);
            int fromSquare = toSquare + 8;
            
            // All 4 promotions
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, KNIGHT));

            promoPawns &= promoPawns - 1;
        }

        return moveList;
    }
    vector<Move> bPawnDoublePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank7Pawns = board.pieces[BLACK][PAWN] & 0x00FF000000000000ULL;
        U64 singlePush = (rank7Pawns >> 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush >> 8) & emptySquares;       // destination must also be empty

        vector<Move> moveList;

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare + 16;
            Move move(fromSquare, toSquare, PAWN, NONE, NONE, false, true);
            moveList.push_back(move); // add to move list

            doublePush &= doublePush - 1; // move onto next least significant bit
        }

        return moveList;
    }
    vector<Move> bPawnCapture(const Board& board) {
        U64 mask2Rank = 0x00000000000000FF00ULL; // mask 7th rank
        U64 leftCaptures  = ((board.pieces[WHITE][PAWN] & ~mask2Rank) >> 7) & board.occupancy[BLACK] & ~MASK_H_FILE;
        U64 rightCaptures = ((board.pieces[WHITE][PAWN] & ~mask2Rank) >> 9) & board.occupancy[BLACK] & ~MASK_A_FILE;
        U64 rank2Pawns = board.pieces[WHITE][PAWN] & mask2Rank;
        U64 leftPromoCaptures = (rank2Pawns >> 7) & board.occupancy[BLACK] & ~MASK_H_FILE;
        U64 rightPromoCaptures = (rank2Pawns >> 9) & board.occupancy[BLACK] & ~MASK_A_FILE;

        vector<Move> moveList;

        // Check left captures
        while (leftCaptures) {
            int toSquare = getLSB(leftCaptures);
            int fromSquare = toSquare + 7;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
        }

        // Check left promotion captures
        while (leftPromoCaptures) {
            int toSquare = getLSB(leftPromoCaptures);
            int fromSquare = toSquare + 7;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));
            
            leftPromoCaptures &= leftPromoCaptures - 1; // move onto next least significant bit
        }
        
        // Check right promotion captures
        while (rightPromoCaptures) {
            int toSquare = getLSB(rightPromoCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);

            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));

            rightPromoCaptures &= rightPromoCaptures - 1; // move onto next least significant bit
        }

        // check for en passant moves
        if (board.enPassantSquare != -1) {   
            if ((board.pieces[WHITE][PAWN] & ~MASK_H_FILE) >> 7 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare + 7, board.enPassantSquare, PAWN, PAWN);
                moveList.push_back(move);
            }
            
            if ((board.pieces[WHITE][PAWN] & ~MASK_A_FILE) >> 9 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare + 9, board.enPassantSquare, PAWN, PAWN);
                moveList.push_back(move);
            }
        }

        return moveList;
    }
    // diagonal sliding move functions
    // direction is based on the orientation of bitboards defined in comments in types.h
    vector<Move> northWestSliding(const Board& board, Piece piece) {
        U64 pieces = board.pieces[board.sideToMove][piece];

        // loop through each of the pieces on bitboard
        vector<Move> moveList;
        while (pieces) {
            int fromSquare = getLSB(pieces);

            U64 currPieceMask = 1ULL << fromSquare;

            // Loop north west 
            while (((currPieceMask << 9) & ~MASK_A_FILE) & ~board.occupancy[board.sideToMove]) {
                int toSquare = fromSquare - 9;
                if (currPieceMask & board.occupancy[board.sideToMove == WHITE ? BLACK : WHITE]) {
                    Piece captured = board.getPieceAt(board.sideToMove == WHITE ? BLACK : WHITE, toSquare);
                    Move move(fromSquare, toSquare, piece, captured);
                    moveList.push_back(move);
                    break; // stop generating moves once opponent piece reached
                }
                else {
                    Move move(fromSquare, toSquare, piece);
                    moveList.push_back(move);
                }
                
                currPieceMask <<= 9;
            }

            pieces &= pieces - 1;
        }

        return moveList;
    }
    vector<Move> northEastSliding(const Board& board, Piece piece) {
        U64 pieces = board.pieces[board.sideToMove][piece];

        // loop through each of the pieces on bitboard
        vector<Move> moveList;
        while (pieces) {
            int fromSquare = getLSB(pieces);

            U64 currPieceMask = 1ULL << fromSquare;

            // Loop north east 
            while (((currPieceMask << 7) & ~MASK_H_FILE) & ~board.occupancy[board.sideToMove]) {
                int toSquare = fromSquare - 7;
                if (currPieceMask & board.occupancy[board.sideToMove == WHITE ? BLACK : WHITE]) {
                    Piece captured = board.getPieceAt(board.sideToMove == WHITE ? BLACK : WHITE, toSquare);
                    Move move(fromSquare, toSquare, piece, captured);
                    moveList.push_back(move);
                    break; // stop generating moves once opponent piece reached
                }
                else {
                    Move move(fromSquare, toSquare, piece);
                    moveList.push_back(move);
                }
                
                currPieceMask <<= 7;
            }

            pieces &= pieces - 1;
        }
        
        return moveList;
    }
    vector<Move> southEastSliding(const Board& board, Piece piece) {
        U64 pieces = board.pieces[board.sideToMove][piece];

        // loop through each of the pieces on bitboard
        vector<Move> moveList;
        while (pieces) {
            int fromSquare = getLSB(pieces);

            U64 currPieceMask = 1ULL << fromSquare;

            // Loop south east 
            while (((currPieceMask >> 7) & ~MASK_H_FILE) & ~board.occupancy[board.sideToMove]) {
                int toSquare = fromSquare + 7;
                if (currPieceMask & board.occupancy[board.sideToMove == WHITE ? BLACK : WHITE]) {
                    Piece captured = board.getPieceAt(board.sideToMove == WHITE ? BLACK : WHITE, toSquare);
                    Move move(fromSquare, toSquare, piece, captured);
                    moveList.push_back(move);
                    break; // stop generating moves once opponent piece reached
                }
                else {
                    Move move(fromSquare, toSquare, piece);
                    moveList.push_back(move);
                }
                
                currPieceMask >>= 7;
            }

            pieces &= pieces - 1;
        }
        
        return moveList;
    }
    vector<Move> southWestSliding(const Board& board, Piece piece) {
        U64 pieces = board.pieces[board.sideToMove][piece];

        // loop through each of the pieces on bitboard
        vector<Move> moveList;
        while (pieces) {
            int fromSquare = getLSB(pieces);

            U64 currPieceMask = 1ULL << fromSquare;

            // Loop south west 
            while (((currPieceMask >> 9) & ~MASK_H_FILE) & ~board.occupancy[board.sideToMove]) {
                int toSquare = fromSquare + 9;
                if (currPieceMask & board.occupancy[board.sideToMove == WHITE ? BLACK : WHITE]) {
                    Piece captured = board.getPieceAt(board.sideToMove == WHITE ? BLACK : WHITE, toSquare);
                    Move move(fromSquare, toSquare, piece, captured);
                    moveList.push_back(move);
                    break; // stop generating moves once opponent piece reached
                }
                else {
                    Move move(fromSquare, toSquare, piece);
                    moveList.push_back(move);
                }
                
                currPieceMask <<= 7;
            }

            pieces &= pieces - 1;
        }
        
        return moveList;
    }
    vector<Move> generatePawnMoves(const Board& board) {

        if (board.sideToMove == WHITE) {
            vector<Move> pawn1 = wPawnSinglePush(board); 
            vector<Move> pawn2 = wPawnDoublePush(board);
            vector<Move> pawn3 = wPawnCapture(board);
            
            pawn1.reserve(pawn1.size() + pawn2.size() + pawn3.size()); 
            pawn1.insert(end(pawn1), begin(pawn2), end(pawn2));  
            pawn1.insert(end(pawn1), begin(pawn3), end(pawn3));
            
            return pawn1;
        }
        else {
            vector<Move> pawn1 = bPawnSinglePush(board);
            vector<Move> pawn2 = bPawnDoublePush(board); 
            vector<Move> pawn3 = bPawnCapture(board);  
            
            pawn1.reserve(pawn1.size() + pawn2.size() + pawn3.size());  
            pawn1.insert(end(pawn1), begin(pawn2), end(pawn2)); 
            pawn1.insert(end(pawn1), begin(pawn3), end(pawn3)); 
            
            return pawn1;
        }
    }
    vector<Move> generateKnightMoves(const Board& board) {
        U64 knights = board.pieces[WHITE][KNIGHT];

        // Go through all the players knights
        vector<Move> moveList;
        while (knights) {
            int fromSquare = getLSB(knights);
            
            // Retrieve knight attack of each player knight and make sure it cannot capture own player pieces
            U64 knightMoves = knightAttacks[fromSquare] & ~board.occupancy[board.sideToMove];
            // Loop through all moves retrieved and store it
            while (knightMoves) {
                int toSquare = getLSB(knightMoves);
                Piece captured = board.getPieceAt(board.sideToMove == WHITE ? BLACK : WHITE, toSquare);
                Move move(fromSquare, toSquare, KNIGHT, captured); // Captured == None if no enemy piece exists at toSquare
                moveList.push_back(move);
                knightMoves &= knightMoves - 1;
            }

            knights &= knights - 1;
        }

        return moveList;
    }
}

namespace MoveGen {

    void init() {
        preComputeKnightMoves();
    }

    vector<Move> MoveGen::generateLegalMoves(const Board& board) {
        vector<Move> moves = generateKnightMoves(board);
        return moves;
    }
}