#pragma once
#include <iostream>
#include <vector>
#include "board.h"
#include "move.h"
#include "types.h"
using namespace std;

namespace MoveGen {
    extern U64 knightAttacks[64];
    extern U64 kingAttacks[64];

    void init();
    U64 getRays(int fromSquare, U64 currPlayerPieces, U64 opponentPieces, const Direction* dirs);
    void generateAllMoves(Color currPlayer, Move* moveList, int& moveCount, Board& board);
    void generateLegalMoves(Board& board, Move* legalMoves, int& legalMovesCount);
}