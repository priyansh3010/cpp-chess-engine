#pragma once
#include "types.h"

struct Move {
    int fromSquare;
    int toSquare;
    Piece pieceType;
    Piece capturedPiece;
    Piece promotionPiece;
    bool isCastle;
    bool isEnPassant;

    Move() = default;

    Move(int from, int to, Piece piece, Piece captured = NONE, 
         Piece promotion = NONE, bool castle = false, bool enPassant = false)
        : fromSquare(from), toSquare(to), pieceType(piece),
          capturedPiece(captured), promotionPiece(promotion),
          isCastle(castle), isEnPassant(enPassant) {}
};