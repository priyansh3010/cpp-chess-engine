#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "evaluation.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"
#include "utils.h"
using namespace std;

const int MAX_DEPTH = 20;
const int MAX_PLY = 128;
Move movePool[256 * MAX_PLY];


// for time management
const int NODES_BEFORE_CHECKING_TIME = 2047;
auto searchStartTime = chrono::steady_clock::now();
int searchAllocatedMs = 5000;
bool stopSearch = false;
int nodesSearched = 0;

// quinscence search variables
const int DELTA_MARGIN = 200;

// move ordering namespace
namespace {
    constexpr int pieceValues[6] = {
        0, // king (shouldn't be captured but just in case)
        900, // queen
        500, // rook
        300, // bishop
        300, // knight
        100 // pawn
    };

    Move killerMoves[2][MAX_PLY];

    int scoreMVVLVA(const Move& move) {
        return pieceValues[move.capturedPiece] - pieceValues[move.pieceType] + 10000;
    }

    void orderMoves(Move* moves, int& moveCount, int currPly, Move bestMove = Move()) {
        // assign scores
        static int scores[256];
        for (int i = 0; i < moveCount; i++) {
            if (moves[i] == bestMove) {
                scores[i] = INF; // always search best move first
            } 
            else if (moves[i].capturedPiece != NONE) {
                scores[i] = scoreMVVLVA(moves[i]);
            }
            /*
            else if (moves[i] == killerMoves[0][currPly]) {
                scores[i] = -1000;
            }
            else if (moves[i] == killerMoves[1][currPly]) {
                scores[i] = -2000;
            }
            */
            else {
                scores[i] = -INF;
            }
        }

        // simple insertion sort
        for (int i = 1; i < moveCount; i++) {
            int key = scores[i];
            Move m = moves[i];
            int j = i - 1;
            while (j >= 0 && scores[j] < key) {
                scores[j + 1] = scores[j];
                moves[j + 1] = moves[j];
                j--;
            }
            scores[j + 1] = key;
            moves[j + 1] = m;
        }
    }
}

// 3-fold repetiion check
namespace {
    bool is3FoldRepetition(Board& board) {
        int count = 0;
        for (int i = board.historyPly - 2; i >= 0; i--) {
            if (board.hash == board.hashHistory[i]) count++;
            if (count == 2) {
                return true; 
            }
        }

        return false;
    }
}

// search namespace
namespace {
    int quiscenceSearch(Board& board, int alpha, int beta, Move* movePool, int plyFromRoot, int& nodesSearched) {
        // check if search time is over
        if ((nodesSearched & NODES_BEFORE_CHECKING_TIME) == 0 && utils::isTimeUp(searchAllocatedMs, searchStartTime)) stopSearch = true;

        // terminate minimax if search over
        if (stopSearch) return 0;

        int static_eval = Evaluation::evaluate(board); // already from white's perspective
        nodesSearched++;

        int best_value = static_eval;
        if (board.sideToMove == WHITE) {
            if (best_value >= beta) return best_value;
            if (best_value > alpha) alpha = best_value;

            Move* moves = movePool + (plyFromRoot * 256);
            int moveCount = 0;
            MoveGen::generateLegalMoves(board, moves, moveCount, true); // generate legal captures only
            
            // order moves
            orderMoves(moves, moveCount, plyFromRoot);
            
            for (int i = 0; i < moveCount; i++) {
                if (moves[i].promotionPiece == NONE) {
                    int capturedValue = pieceValues[moves[i].capturedPiece];
                    if (static_eval + capturedValue + DELTA_MARGIN < alpha) continue;
                }

                MoveInfo moveInfo = board.makeMove(moves[i]);
                int score = quiscenceSearch(board, alpha, beta, movePool, plyFromRoot + 1, nodesSearched);
                board.unMakeMove(moveInfo);
                
                if (score >= beta) return score;
                if (score > best_value) best_value = score;
                if (score > alpha) alpha = score;
            }
        }
        else {
            // mirror logic, but flipped for black
            if (best_value <= alpha) return best_value;
            if (best_value < beta) beta = best_value;
            
            Move* moves = movePool + (plyFromRoot * 256);
            int moveCount = 0;
            MoveGen::generateLegalMoves(board, moves, moveCount, true); // generate legal captures only

            // order moves
            orderMoves(moves, moveCount, plyFromRoot);
            
            for (int i = 0; i < moveCount; i++) {
                if (moves[i].promotionPiece == NONE) {
                    int capturedValue = pieceValues[moves[i].capturedPiece];
                    if (static_eval - capturedValue - DELTA_MARGIN > beta) continue;
                }

                MoveInfo moveInfo = board.makeMove(moves[i]);
                int score = quiscenceSearch(board, alpha, beta, movePool, plyFromRoot + 1, nodesSearched);
                board.unMakeMove(moveInfo);

                if (score <= alpha) return score;
                if (score < best_value) best_value = score;
                if (score < beta) beta = score;
            }
        }

        return best_value;
    }

    int minimax(Board& board, int depth, int alpha, int beta, Move* movePool, int plyFromRoot, int& nodesSearched) {
        
        bool maximizingPlayer = board.sideToMove == WHITE;
        // check if search time is over
        if ((nodesSearched & NODES_BEFORE_CHECKING_TIME) == 0 && utils::isTimeUp(searchAllocatedMs, searchStartTime)) stopSearch = true;

        // terminate minimax if search over
        if (stopSearch) return 0;
        
        // increment nodesSearched
        nodesSearched++;

        // evaluate final position
        if (depth == 0) 
            return quiscenceSearch(board, alpha, beta, movePool, plyFromRoot, nodesSearched);

        Move* moves = movePool + (plyFromRoot * 256);
        int moveCount = 0;
        MoveGen::generateLegalMoves(board, moves, moveCount); // generate legal moves of current player

        // check if game is stalemated or checkmated
        if (moveCount == 0) {
            // checkmated
            if (board.isKingInCheck(board.sideToMove)) {
                return board.sideToMove == WHITE ? -INF + plyFromRoot : INF - plyFromRoot;
            }
            // stalemated
            return 0;
        }

        // enforce 50-move draw
        if (board.halfMoveClock >= 50) return 0;

        // enforce draw by insufficient material
        if (board.isInsufficientMaterial()) return 0;

        // move ordering
        orderMoves(moves, moveCount, plyFromRoot);

        // if whites turn
        if (maximizingPlayer) {
            int bestEval = -INF;
            for (int i = 0; i < moveCount; i++) {
                Move& move = moves[i];
                MoveInfo moveInfo = board.makeMove(move);
                
                // check for 3=fold repetition
                if (is3FoldRepetition(board)) {
                    board.unMakeMove(moveInfo);
                    return 0;
                }

                int currEval = minimax(board, depth - 1, alpha, beta, movePool, plyFromRoot + 1, nodesSearched);
                board.unMakeMove(moveInfo);

                /*
                // update killer moves
                if (currEval >= beta) {
                    killerMoves[1][plyFromRoot] = killerMoves[0][plyFromRoot];
                    killerMoves[0][plyFromRoot] = move;
                    
                    return currEval;
                }
                */

                bestEval = max(bestEval, currEval);
                
                // alpha-beta pruning
                alpha = max(alpha, currEval);
                if (beta <= alpha) break;
            }
            return bestEval;
        }
        // if blacks turn
        else {
            int bestEval = INF;
            for (int i = 0; i < moveCount; i++) {
                Move& move = moves[i];
                MoveInfo moveInfo = board.makeMove(move);

                // check for 3=fold repetition
                if (is3FoldRepetition(board)) {
                    board.unMakeMove(moveInfo);
                    return 0;
                }

                // recursively call minimax
                int currEval = minimax(board, depth - 1, alpha, beta, movePool, plyFromRoot + 1, nodesSearched);
                board.unMakeMove(moveInfo);

                /*
                // update killer moves
                if (currEval <= alpha) {
                    if (move.capturedPiece == NONE && move.promotionPiece == NONE) {
                        killerMoves[1][plyFromRoot] = killerMoves[0][plyFromRoot];
                        killerMoves[0][plyFromRoot] = move;
                    }
                    
                    return currEval;
                }
                */

                bestEval = min(bestEval, currEval);

                // beta pruning
                beta = min(beta, currEval);
                if (beta <= alpha) break;
            }
            return bestEval;
        }
    }

    Move searchRoot(Board& board, int depth, Move* moves, int& moveCount, int& nodesSearched, Move bestMove = Move()) {
        bool maximizing = board.sideToMove == WHITE;

        int bestEval = maximizing ? -INF : INF;
        
        // move ordering
        orderMoves(moves, moveCount, 0, bestMove);
        
        // loop through all moves
        for (int i = 0; i < moveCount; i++) {
            Move& move = moves[i];
            MoveInfo moveInfo = board.makeMove(move);
            
            int currEval;
            // check for 3=fold repetition
            if (is3FoldRepetition(board)) {
                board.unMakeMove(moveInfo);
                currEval = 0;
            }
            else {
                currEval = minimax(board, depth - 1, -INF, INF, movePool, 1, nodesSearched);
                board.unMakeMove(moveInfo);
            }

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
    Move getBestMove(Board& board, int allocatedMS) {
        // get all legal moves on the board
        Move* moves = movePool;
        int moveCount = 0;
        MoveGen::generateLegalMoves(board, moves, moveCount);

        // intilize and set all values to time search
        searchStartTime = chrono::steady_clock::now();
        searchAllocatedMs = allocatedMS;
        stopSearch = false;
        nodesSearched = 0;

        Move bestMove = moves[0];
        // iterative deepening
        int maxDepthReached = 1;
        for (int depth = 1; depth <= MAX_DEPTH; depth++) {
            Move candidateMove;
            if (depth == 1) candidateMove = searchRoot(board, depth, moves, moveCount, nodesSearched); 
            else candidateMove = searchRoot(board, depth, moves, moveCount, nodesSearched, bestMove); 
            
            // if time finished, discard previous depth results
            // because not all nodes explored
            if (stopSearch) break;

            maxDepthReached = depth;
            bestMove = candidateMove;
        }
        
        // pass extra info for engine stats
        cout << "info depth " << maxDepthReached << " score cp " << Evaluation::evaluate(board) << " nodes " << nodesSearched << endl;
        return bestMove;
    }
}