#pragma once
#include <string>
#include "types.h"
#include "move.h"
#include "moveinfo.h"
using namespace std;

struct Board {
    U64 pieces[2][6]; // one for all different types of pieces for black and white
    U64 occupancy[3]; // white, black, all

    Color sideToMove;
    int enPassantSquare;
    int castlingRights;
    int halfMoveClock;
    int fullMoveNumber;

    void loadFEN(string FEN);
    void init(string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    void printBoard();
    Piece getPieceAt(Color color, int square) const;
    MoveInfo makeMove(Move move);
    void unMakeMove(MoveInfo moveInfo);
    bool isKingInCheck();
    bool isSquareAttacked(int toSquare);
};