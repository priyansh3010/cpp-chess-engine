#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"

U64 perft(Board& board, int depth, Move* movePool);
int perftDivide(Board& board, int depth, Move* movePool);