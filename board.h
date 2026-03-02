#pragma once
#include "types.h"

struct Board {
    U64 pieces[2][6]; // one for all different types of pieces for black and white
    U64 occupancy[3]; // white, black, all

    Color sideToMove;
    int enPassantSquare;
    int castlingRights;
    int halfMoveClock;
    int fullMoveNumber;
};