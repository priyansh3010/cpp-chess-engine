#pragma once
#include <iostream>
#include <vector>
#include "board.h"
#include "movegen.h"
#include "move.h"
#include "types.h"
using namespace std;

namespace {
    // Pawn move functions
    vector<Move> wPawnSinglePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 singlePush = (board.pieces[WHITE][PAWN] << 8) & emptySquares; // all squares white pawns can move to

        vector<Move> moveList;

        // Find all 1 bits to generate single push moves
        while (singlePush) {
            int toSquare = getLSB(singlePush);
            int fromSquare = toSquare - 8;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            singlePush &= singlePush - 1; // move onto next least significant bit
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
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            doublePush &= doublePush - 1; // move onto next least significant bit
        }

        return moveList;
    }
    vector<Move> wPawnCapture(const Board& board) {
        U64 leftCaptures  = (board.pieces[WHITE][PAWN] << 7) & board.occupancy[BLACK] & ~0x8080808080808080ULL; // mask H file
        U64 rightCaptures = (board.pieces[WHITE][PAWN] << 9) & board.occupancy[BLACK] & ~0x0101010101010101ULL; // mask A file

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

        return moveList;
    }
    vector<Move> bPawnSinglePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 singlePush = (board.pieces[BLACK][PAWN] >> 8) & emptySquares; // all squares black pawns can move to

        vector<Move> moveList;

        // Find all 1 bits to generate single push moves
        while (singlePush) {
            int toSquare = getLSB(singlePush);
            int fromSquare = toSquare + 8;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            singlePush &= singlePush - 1; // move onto next least significant bit
        }

        return moveList;
    }
    vector<Move> bPawnDoublePush(const Board& board) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank7Pawns = board.pieces[BLACK][PAWN] & 0x000000000000FF00ULL;
        U64 singlePush = (rank7Pawns >> 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush >> 8) & emptySquares;       // destination must also be empty

        vector<Move> moveList;

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare + 16;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            doublePush &= doublePush - 1; // move onto next least significant bit
        }

        return moveList;
    }
    vector<Move> bPawnCapture(const Board& board) {
        U64 leftCaptures  = (board.pieces[BLACK][PAWN] >> 7) & board.occupancy[BLACK] & ~0x8080808080808080ULL; // mask H file
        U64 rightCaptures = (board.pieces[BLACK][PAWN] >> 9) & board.occupancy[BLACK] & ~0x0101010101010101ULL; // mask A file

        vector<Move> moveList;

        // Check left captures
        while (leftCaptures) {
            int toSquare = getLSB(leftCaptures);
            int fromSquare = toSquare + 7;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
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
            vector<Move> pawn1 = wPawnSinglePush(board);
            vector<Move> pawn2 = wPawnDoublePush(board);
            vector<Move> pawn3 = wPawnCapture(board);
            
            pawn1.reserve(pawn1.size() + pawn2.size() + pawn3.size());
            pawn1.insert(end(pawn1), begin(pawn2), end(pawn2));
            pawn1.insert(end(pawn1), begin(pawn3), end(pawn3));
            
            return pawn1;
        }
    }

    
}

namespace MoveGen {
    vector<Move> MoveGen::generateLegalMoves(const Board& board) {
        return generatePawnMoves(board);
    }
}