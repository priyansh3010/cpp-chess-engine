#pragma once
#include <iostream>
#include <vector>
#include "board.h"
#include "move.h"
#include "types.h"
using namespace std;

namespace MoveGen {

    struct Direction { int shift; U64 noWrap; };

    void init();
    U64 getRays(int fromSquare, U64 currPlayerPieces, U64 opponentPieces, const Direction* dirs);
    vector<Move> generateAllMoves(Color currPlayer, Board& board);
    vector<Move> generateLegalMoves(Board& board);
}