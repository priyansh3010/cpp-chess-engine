#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"

const int MAX_DEPTH = 5;
Move movePool[256 * (MAX_DEPTH)]; // +1 for the getBestMove level

namespace {
    int evaluate(Board& board) {
        // simple evaluation function to count all pieces
        int currEval = 0;

        // go through all the white pieces
        U64 whitePieces = board.occupancy[WHITE];
        while (whitePieces) {
            int currIndex = getLSB(whitePieces);
            Piece currPiece = board.getPieceAt(WHITE, currIndex);
            if (currPiece == QUEEN) currEval += 9;
            if (currPiece == ROOK) currEval += 5;
            if (currPiece == BISHOP) currEval += 3;
            if (currPiece == KNIGHT) currEval += 3;
            if (currPiece == PAWN) currEval += 1;

            whitePieces &= whitePieces - 1;
        }

        // go through all the black pieces
        U64 blackPieces = board.occupancy[BLACK];
        while (blackPieces) {
            int currIndex = getLSB(blackPieces);
            Piece currPiece = board.getPieceAt(BLACK, currIndex);

            // add all piece values
            if (currPiece == QUEEN) currEval -= 9;
            if (currPiece == ROOK) currEval -= 5;
            if (currPiece == BISHOP) currEval -= 3;
            if (currPiece == KNIGHT) currEval -= 3;
            if (currPiece == PAWN) currEval -= 1;

            blackPieces &= blackPieces - 1;
        }

        return currEval;
    }

    int minimax(Board& board, int depth, int alpha, int beta, Move* movePool, int plyFromRoot) {
        bool maximizingPlayer = board.sideToMove == WHITE;
        // evaluate final position
        if (depth == 0) return evaluate(board);

        Move* moves = movePool + (plyFromRoot * 256);
        int moveCount = 0;
        MoveGen::generateLegalMoves(board, moves, moveCount); // generate legal opponent moves

        // check if game is stalemated or checkmated
        if (moveCount == 0) {
            // checkmated
            if (board.isKingInCheck(board.sideToMove)) {
                return board.sideToMove == WHITE ? INT_MIN : INT_MAX;
            }
            // stalemated
            return 0;
        }

        // if whites turn
        if (maximizingPlayer) {
            int bestEval = INT_MIN;
            for (int i = 0; i < moveCount; i++) {
                Move& move = moves[i];
                MoveInfo moveInfo = board.makeMove(move);
                int currEval = minimax(board, depth - 1, alpha, beta, movePool, plyFromRoot + 1);
                board.unMakeMove(moveInfo);
                bestEval = max(bestEval, currEval);
                
                // alpha-beta pruning
                alpha = max(alpha, currEval);
                if (beta <= alpha) break;
            }
            return bestEval;
        }
        // if blacks turn
        else {
            int bestEval = INT_MAX;
            for (int i = 0; i < moveCount; i++) {
                Move& move = moves[i];
                MoveInfo moveInfo = board.makeMove(move);
                int currEval = minimax(board, depth - 1, alpha, beta, movePool, plyFromRoot + 1);
                board.unMakeMove(moveInfo);
                bestEval = min(bestEval, currEval);

                // beta pruning
                beta = min(beta, currEval);
                if (beta <= alpha) break;
            }
            return bestEval;
        }
    }
}

namespace Engine {
    Move getBestMove(Board& board) {
        Move* moves = movePool;
        int moveCount = 0;
        MoveGen::generateLegalMoves(board, moves, moveCount);
        bool maximizing = board.sideToMove == WHITE;
        
        Move bestMove = moves[0];
        int bestEval = maximizing ? INT_MIN : INT_MAX;
        for (int i = 0; i < moveCount; i++) {
            Move& move = moves[i];
            MoveInfo moveInfo = board.makeMove(move);
            int currEval = minimax(board, 4, INT_MIN, INT_MAX, movePool, 1);
            board.unMakeMove(moveInfo);
            
            if (maximizing) {
                if (bestEval < currEval) {
                    bestEval = currEval;
                    bestMove = move;
                } 
            }
            else {
                if (bestEval > currEval) {
                    bestEval = currEval;
                    bestMove = move;
                }     
            }
        }
        
        return bestMove;
    }
}