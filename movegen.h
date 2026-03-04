#pragma once
#include <iostream>
#include <vector>
#include "board.h"
#include "move.h"
using namespace std;

namespace MoveGen {
    vector<Move> generateLegalMoves(const Board& board);
}