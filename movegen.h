#pragma once
#include <iostream>
#include <vector>
#include "board.h"
#include "move.h"
#include "types.h"
using namespace std;

namespace MoveGen {

    void init();
    vector<Move> generateAllMoves(Board& board);
    vector<Move> generateLegalMoves(Board& board);
}