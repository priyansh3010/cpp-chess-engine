#pragma once
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"
#include "utils.h"
using namespace std;


U64 perft(Board& board, int depth, Move* movePool) {
    if (depth == 0) return 1;
    
    Move* moves = movePool + (depth * 256);
    int moveCount = 0;
    MoveGen::generateLegalMoves(board, moves, moveCount);
    
    U64 nodes = 0;
    for (int i = 0; i < moveCount; i++) {
        Move& move = moves[i];
        MoveInfo moveInfo = board.makeMove(move);
        nodes += perft(board, depth - 1, movePool);
        board.unMakeMove(moveInfo);
    }
    return nodes;
}

int perftDivide(Board& board, int depth, Move* movePool) {
    Move* moves = movePool + (depth * 256);
    int moveCount = 0;
    MoveGen::generateLegalMoves(board, moves, moveCount);
    
    uint64_t total = 0;
    for (int i = 0; i < moveCount; i++) {
        Move& move = moves[i];
        MoveInfo moveInfo = board.makeMove(move);
        uint64_t count = perft(board, depth - 1, movePool);
        board.unMakeMove(moveInfo);

        cout << utils::moveToString(move);
        cout << ": " << count << endl;
        total += count;
    }
    cout << "\nTotal: " << total << endl;
    return total;
}