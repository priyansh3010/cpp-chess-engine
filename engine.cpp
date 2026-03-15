#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"

namespace {
    int evaluate(Board& board) {
        // simple evaluation function to count all pieces
        int currEval = 0;
        U64 whitePieces = board.occupancy[WHITE];
        // go through all the white pieces
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

        U64 blackPieces = board.occupancy[BLACK];
        // go through all the white pieces
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

    int minimax(Board& board, int depth, int alpha, int beta) {
        bool maximizingPlayer = board.sideToMove == WHITE;
        // evaluate final position
        if (depth == 0) {
            return evaluate(board);
        }

        vector<Move> moves = MoveGen::generateLegalMoves(board); // generate all opponent moves

        // check if game is stalemated or checkmated
        if (moves.size() == 0) {
            board.sideToMove = board.sideToMove == WHITE ? BLACK : WHITE;
            // checkmated
            if (board.isKingInCheck()) {
                board.sideToMove = board.sideToMove == WHITE ? BLACK : WHITE;
                return board.sideToMove == WHITE ? INT_MIN : INT_MAX;
            }
            // stalemated
            board.sideToMove = board.sideToMove == WHITE ? BLACK : WHITE;
            return 0;
        }

        // if whites turn
        if (maximizingPlayer) {
            int bestEval = INT_MIN;
            for (const Move& move : moves) {
                MoveInfo moveInfo = board.makeMove(move);
                int currEval = minimax(board, depth - 1, alpha, beta);
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
            for (const Move& move : moves) {
                MoveInfo moveInfo = board.makeMove(move);
                int currEval = minimax(board, depth - 1, alpha, beta);
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
        vector<Move> moves = MoveGen::generateLegalMoves(board);
        bool maximizing = board.sideToMove == WHITE;
        
        Move bestMove = moves[0];
        int bestEval = maximizing ? INT_MIN : INT_MAX;
        for (const Move& move : moves) {
            MoveInfo moveInfo = board.makeMove(move);
            int currEval = minimax(board, 4, INT_MIN, INT_MAX);
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