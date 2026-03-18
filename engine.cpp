#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"
#include "utils.h"
using namespace std;

const int MAX_DEPTH = 10;
Move movePool[256 * (MAX_DEPTH)]; // +1 for the getBestMove level

// for time management
auto searchStartTime = chrono::steady_clock::now();
int searchAllocatedMs = 5000;
bool stopSearch = false;
int nodesSearched = 0;

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

    int minimax(Board& board, int depth, int alpha, int beta, Move* movePool, int plyFromRoot, int& nodesSearched) {
        bool maximizingPlayer = board.sideToMove == WHITE;
        // check if search time is over
        if ((nodesSearched & 2047) == 0 && utils::isTimeUp(searchAllocatedMs, searchStartTime)) stopSearch = true;

        // terminate minimax if search over
        if (stopSearch) return 0;
        
        // increment nodesSearched
        nodesSearched++;

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
                int currEval = minimax(board, depth - 1, alpha, beta, movePool, plyFromRoot + 1, nodesSearched);
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
                int currEval = minimax(board, depth - 1, alpha, beta, movePool, plyFromRoot + 1, nodesSearched);
                board.unMakeMove(moveInfo);
                bestEval = min(bestEval, currEval);

                // beta pruning
                beta = min(beta, currEval);
                if (beta <= alpha) break;
            }
            return bestEval;
        }
    }

    Move searchRoot(Board& board, int depth, Move* moves, int& moveCount) {
        bool maximizing = board.sideToMove == WHITE;
        
        int nodesSearched = 0;
        Move bestMove = moves[0];
        int bestEval = maximizing ? INT_MIN : INT_MAX;
        for (int i = 0; i < moveCount; i++) {
            Move& move = moves[i];
            MoveInfo moveInfo = board.makeMove(move);
            int currEval = minimax(board, depth - 1, INT_MIN, INT_MAX, movePool, 1, nodesSearched);
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

namespace Engine {
    Move getBestMove(Board& board) {
        Move* moves = movePool;
        int moveCount = 0;
        MoveGen::generateLegalMoves(board, moves, moveCount);

        searchStartTime = chrono::steady_clock::now();
        searchAllocatedMs = 5000;
        stopSearch = false;
        nodesSearched = 0;

        Move bestMove = moveCount != 0 ? moves[0] : Move{};

        for (int depth = 1; depth <= MAX_DEPTH; depth++) {
            Move candidateMove = searchRoot(board, depth, moves, moveCount);

            if (stopSearch) break;

            bestMove = candidateMove;
        }

        return bestMove;
    }
}