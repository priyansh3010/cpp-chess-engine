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

    Move() : fromSquare(-1), toSquare(-1), pieceType(NONE), capturedPiece(NONE), 
             promotionPiece(NONE), isCastle(false), isEnPassant(false) {}

    Move(int from, int to, Piece piece, Piece captured = NONE, 
         Piece promotion = NONE, bool castle = false, bool enPassant = false)
        : fromSquare(from), toSquare(to), pieceType(piece),
          capturedPiece(captured), promotionPiece(promotion),
          isCastle(castle), isEnPassant(enPassant) {}
    
    bool operator==(const Move& other) const {
        return fromSquare == other.fromSquare && toSquare == other.toSquare && promotionPiece == other.promotionPiece;
    }
};