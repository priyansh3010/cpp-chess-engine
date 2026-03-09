#pragma once
#include "types.h"
#include "move.h"

struct MoveInfo {
    Move move;
    int prevEnPassantSquare;
    int prevCastling;
    int prevHalfMoveClock;
    int prevFullMoveNumber;

    MoveInfo(Move m, int prevEnPassant, int prevCastling, int prevHalfMove, 
         int prevFullMove)
        : move(m), prevEnPassantSquare(prevEnPassant), prevCastling(prevCastling),
          prevHalfMoveClock(prevHalfMove), prevFullMoveNumber(prevFullMove) {}
};