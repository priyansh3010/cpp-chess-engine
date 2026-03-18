#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"
#include "utils.h"

string utils::moveToString(Move move) {
    string files[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    string ranks[] = {"1", "2", "3", "4", "5", "6", "7", "8"};

    string moveString = files[move.fromSquare % 8] + ranks[move.fromSquare / 8] + 
                        files[move.toSquare % 8] + ranks[move.toSquare / 8];

    if (move.promotionPiece != NONE) {
        if (move.promotionPiece == QUEEN) moveString += "q";
        else if (move.promotionPiece == ROOK) moveString += "r";
        else if (move.promotionPiece == BISHOP) moveString += "b";
        else if (move.promotionPiece == KNIGHT) moveString += "n";
    }

    return moveString;
}

Move utils::stringToMove(Board& board, string moveStr) {
    // parse squares
    int fromFile = moveStr[0] - 'a';
    int fromRank = moveStr[1] - '1';
    int toFile = moveStr[2] - 'a';
    int toRank = moveStr[3] - '1';

    Square from = (Square)(fromRank * 8 + fromFile);
    Square to = (Square)(toRank * 8 + toFile);

    // check for promotion piece (5th char)
    Piece promoPiece = Piece::NONE;
    if (moveStr.size() == 5) {
        switch (moveStr[4]) {
            case 'q': promoPiece = Piece::QUEEN;  break;
            case 'r': promoPiece = Piece::ROOK;   break;
            case 'b': promoPiece = Piece::BISHOP; break;
            case 'n': promoPiece = Piece::KNIGHT; break;
        }
    }

    // match against generated moves
    Move moves[256];
    int moveCount = 0;
    MoveGen::generateAllMoves(board.sideToMove, moves, moveCount, board);
    for (int i = 0; i < moveCount; i++) {
        Move& move = moves[i];
        if (move.fromSquare == from && move.toSquare == to) {
            if (promoPiece == Piece::NONE || move.promotionPiece == promoPiece)
                return move;
        }
    }

    return Move{}; // null move if not found
}