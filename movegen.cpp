#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"
using namespace std;

// defines direction struct for sliding piece movegen
namespace {
    const MoveGen::Direction DIAG_DIRS[4] = {
        { +9, ~MASK_A_FILE }, // north west
        { +7, ~MASK_H_FILE }, // north east
        { -9, ~MASK_H_FILE }, // south east
        { -7, ~MASK_A_FILE }  // south west
    };
    
    const MoveGen::Direction STRAIGHT_DIRS[4] = {
        { +8, ~0ULL },        // north
        { -8, ~0ULL },        // south 
        { +1, ~MASK_A_FILE }, // west
        { -1, ~MASK_H_FILE }  // east
    };
}   

// precompute king and knight pieces attack tables
namespace {
    U64 knightAttacks[64];
    U64 bishopAttacks[64];
    U64 rookAttacks[64];
    U64 queenAttacks[64];
    U64 kingAttacks[64];

    // pre-compute knight moves
    // direction is based on the orientation of bitboards defined in comments in types.h
    void preComputeKnightMoves() {
        for (int i = 0; i < 64; i++) {
            U64 currIndex = 1ULL << i;
            U64 northWestWest = (currIndex << 6) & ~MASK_G_H_FILE;
            U64 northNorthWest = (currIndex << 15) & ~MASK_H_FILE;
            U64 northNorthEast = (currIndex << 17) & ~MASK_A_FILE;
            U64 northEastEast = (currIndex << 10) & ~MASK_A_B_FILE;
            U64 southEastEast = (currIndex >> 6) & ~MASK_A_B_FILE;
            U64 southSouthEast = (currIndex >> 15) & ~MASK_A_FILE;
            U64 southSouthWest = (currIndex >> 17) & ~MASK_H_FILE;
            U64 southWestWest = (currIndex >> 10) & ~MASK_G_H_FILE;
            
            U64 currMoveMap = northWestWest | northNorthWest | northNorthEast | northEastEast
            | southWestWest | southSouthWest | southSouthEast | southEastEast;
            
            knightAttacks[i] = currMoveMap;
        }
    }
    
    // pre-compute knight moves
    // direction is based on the orientation of bitboards defined in comments in types.h
    void preComputeKingMoves() {
        for (int i = 0; i < 64; i++) {
            U64 currIndex = 1ULL << i;
            U64 west = (currIndex << 1) & ~MASK_A_FILE;
            U64 northWest = (currIndex << 9) & ~MASK_A_FILE;
            U64 north = (currIndex << 8);
            U64 northEast = (currIndex << 7) & ~MASK_H_FILE;
            U64 east = (currIndex >> 1) & ~MASK_H_FILE;
            U64 southEast = (currIndex >> 9) & ~MASK_H_FILE;
            U64 south = (currIndex >> 8);
            U64 southWest = (currIndex >> 7) & ~MASK_A_FILE;
            
            U64 currMoveMap = west | northWest | north | northEast
                            | east | southEast | south | southWest;
            
            kingAttacks[i] = currMoveMap;
        }
    }
}

// Pawn and sliding pieces move functions
namespace {
    void wPawnSinglePush(const Board& board, vector<Move>& moveList) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank7Mask = 0x00FF000000000000ULL; // Will highlight all pawns on 7th rank

        U64 singlePush = ((board.pieces[WHITE][PAWN] & ~rank7Mask) << 8) & emptySquares; // all squares white pawns can move to
        
        U64 rank7Pawns = board.pieces[WHITE][PAWN] & rank7Mask; // all squares 7th rank white pawns can move to
        U64 promoPawns = (rank7Pawns << 8) & emptySquares;
        
        // Find all 1 bits to generate single push moves
        while (singlePush) {
            int toSquare = getLSB(singlePush);
            int fromSquare = toSquare - 8;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list
            
            singlePush &= singlePush - 1; // move onto next least significant bit
        }
        
        // all promo moves possible
        while (promoPawns) {
            int toSquare = getLSB(promoPawns);
            int fromSquare = toSquare - 8;
            
            // All 4 promotions
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, ROOK)); 
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, BISHOP));             
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, KNIGHT)); 
            
            promoPawns &= promoPawns - 1;
        }
    }
    void wPawnDoublePush(const Board& board, vector<Move>& moveList) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank2Pawns = board.pieces[WHITE][PAWN] & 0x000000000000FF00ULL;
        U64 singlePush = (rank2Pawns << 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush << 8) & emptySquares;       // destination must also be empty

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare - 16;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            doublePush &= doublePush - 1; // move onto next least significant bit
        }
    }
    void wPawnCapture(const Board& board, vector<Move>& moveList) {
        U64 mask7Rank = 0x00FF000000000000ULL; // mask 7th rank
        U64 leftCaptures  = ((board.pieces[WHITE][PAWN] & ~mask7Rank) << 7) & board.occupancy[BLACK] & ~MASK_H_FILE;
        U64 rightCaptures = ((board.pieces[WHITE][PAWN] & ~mask7Rank) << 9) & board.occupancy[BLACK] & ~MASK_A_FILE;
        U64 rank7Pawns = board.pieces[WHITE][PAWN] & mask7Rank;
        U64 leftPromoCaptures = (rank7Pawns << 7) & board.occupancy[BLACK] & ~MASK_H_FILE;
        U64 rightPromoCaptures = (rank7Pawns << 9) & board.occupancy[BLACK] & ~MASK_A_FILE;

        // Check left captures
        while (leftCaptures) {
            int toSquare = getLSB(leftCaptures);
            int fromSquare = toSquare - 7;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare - 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
        }

        // Check left promotion captures
        while (leftPromoCaptures) {
            int toSquare = getLSB(leftPromoCaptures);
            int fromSquare = toSquare - 7;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);

            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));
            
            leftPromoCaptures &= leftPromoCaptures - 1; // move onto next least significant bit
        }
        
        // Check right promotion captures
        while (rightPromoCaptures) {
            int toSquare = getLSB(rightPromoCaptures);
            int fromSquare = toSquare - 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);

            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));

            rightPromoCaptures &= rightPromoCaptures - 1; // move onto next least significant bit
        }

        // check for en passant moves
        if (board.enPassantSquare != -1) {   
            if ((board.pieces[WHITE][PAWN] & ~MASK_A_FILE) << 7 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare - 7, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList.push_back(move);
            }
            
            if ((board.pieces[WHITE][PAWN] & ~MASK_H_FILE) << 9 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare - 9, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList.push_back(move);
            }
        }
    }
    void bPawnSinglePush(const Board& board, vector<Move>& moveList) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank2Mask = 0x000000000000FF00ULL; // Will highlight all pawns on 2nd rank

        U64 singlePush = ((board.pieces[BLACK][PAWN] & ~rank2Mask) >> 8) & emptySquares; // all squares white pawns can move to
        
        U64 rank2Pawns = board.pieces[BLACK][PAWN] & rank2Mask; // all squares 2nd rank white pawns can move to
        U64 promoPawns = (rank2Pawns >> 8) & emptySquares;

        // Find all 1 bits to generate single push moves
        while (singlePush) {
            int toSquare = getLSB(singlePush);
            int fromSquare = toSquare + 8;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            singlePush &= singlePush - 1; // move onto next least significant bit
        }

        while (promoPawns) {
            int toSquare = getLSB(promoPawns);
            int fromSquare = toSquare + 8;
            
            // All 4 promotions
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, NONE, KNIGHT));

            promoPawns &= promoPawns - 1;
        }
    }
    void bPawnDoublePush(const Board& board, vector<Move>& moveList) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank7Pawns = board.pieces[BLACK][PAWN] & 0x00FF000000000000ULL;
        U64 singlePush = (rank7Pawns >> 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush >> 8) & emptySquares;       // destination must also be empty

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare + 16;
            Move move(fromSquare, toSquare, PAWN);
            moveList.push_back(move); // add to move list

            doublePush &= doublePush - 1; // move onto next least significant bit
        }
    }
    void bPawnCapture(const Board& board, vector<Move>& moveList) {
        U64 mask2Rank = 0x00000000000000FF00ULL; // mask 7th rank
        U64 leftCaptures  = ((board.pieces[BLACK][PAWN] & ~mask2Rank) >> 7) & board.occupancy[WHITE] & ~MASK_A_FILE;
        U64 rightCaptures = ((board.pieces[BLACK][PAWN] & ~mask2Rank) >> 9) & board.occupancy[WHITE] & ~MASK_H_FILE;
        U64 rank2Pawns = board.pieces[BLACK][PAWN] & mask2Rank;
        U64 leftPromoCaptures = (rank2Pawns >> 7) & board.occupancy[WHITE] & ~MASK_A_FILE;
        U64 rightPromoCaptures = (rank2Pawns >> 9) & board.occupancy[WHITE] & ~MASK_H_FILE;

        // Check left captures
        while (leftCaptures) {
            int toSquare = getLSB(leftCaptures);
            int fromSquare = toSquare + 7;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList.push_back(move); // add to move list

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
        }

        // Check left promotion captures
        while (leftPromoCaptures) {
            int toSquare = getLSB(leftPromoCaptures);
            int fromSquare = toSquare + 7;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));
            
            leftPromoCaptures &= leftPromoCaptures - 1; // move onto next least significant bit
        }
        
        // Check right promotion captures
        while (rightPromoCaptures) {
            int toSquare = getLSB(rightPromoCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);

            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, ROOK));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, BISHOP));
            moveList.push_back(Move(fromSquare, toSquare, PAWN, captured, KNIGHT));

            rightPromoCaptures &= rightPromoCaptures - 1; // move onto next least significant bit
        }

        // check for en passant moves
        if (board.enPassantSquare != -1) {   
            if ((board.pieces[BLACK][PAWN] & ~MASK_H_FILE) >> 7 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare + 7, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList.push_back(move);
            }
            
            if ((board.pieces[BLACK][PAWN] & ~MASK_A_FILE) >> 9 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare + 9, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList.push_back(move);
            }
        }
    }
    void generateSlidingMoves(const Board& board, Piece piece, const MoveGen::Direction* dirs, vector<Move>& moveList, Color currPlayer) {
        Color enemy = currPlayer == WHITE ? BLACK : WHITE;
        U64 currColorPieces = board.pieces[currPlayer][piece];

        // loop through all specified piece of the player
        while (currColorPieces) {
            int fromSquare = getLSB(currColorPieces);

            // get all possible attack squares of the one piece
            U64 currAttacks = MoveGen::getRays(fromSquare, board.occupancy[currPlayer], board.occupancy[enemy], dirs);

            // go through each attack and create a move
            while (currAttacks) {
                int toSquare = getLSB(currAttacks);
                Piece captured = board.getPieceAt(enemy, toSquare);
                Move move(fromSquare, toSquare, piece, captured);
                moveList.push_back(move);

                currAttacks &= currAttacks - 1;
            }

            currColorPieces &= currColorPieces - 1;
        }
    }
}
// move generation function for each individual type of piece
namespace {
    void generatePawnMoves(const Board& board, vector<Move>& moveList, Color currPlayer) {

        if (currPlayer == WHITE) {
            wPawnSinglePush(board, moveList); 
            wPawnDoublePush(board, moveList);
            wPawnCapture(board, moveList);
        }
        else {
            bPawnSinglePush(board, moveList);
            bPawnDoublePush(board, moveList); 
            bPawnCapture(board, moveList);  
        }
    }
    void generateKnightMoves(const Board& board, vector<Move>& moveList, Color currPlayer) {
        U64 knights = board.pieces[currPlayer][KNIGHT];

        // Go through all the players knights
        while (knights) {
            int fromSquare = getLSB(knights);
            
            // Retrieve knight attack of each player knight and make sure it cannot capture own player pieces
            U64 knightMoves = knightAttacks[fromSquare] & ~board.occupancy[currPlayer];
            // Loop through all moves retrieved and store it
            while (knightMoves) {
                int toSquare = getLSB(knightMoves);
                Piece captured = board.getPieceAt(currPlayer == WHITE ? BLACK : WHITE, toSquare);
                Move move(fromSquare, toSquare, KNIGHT, captured); // Captured == None if no enemy piece exists at toSquare
                moveList.push_back(move);
                knightMoves &= knightMoves - 1;
            }

            knights &= knights - 1;
        }
    }
    void generateBishopMoves(const Board& board, vector<Move>& moveList, Color currPlayer) {
        generateSlidingMoves(board, BISHOP, DIAG_DIRS, moveList, currPlayer);
    }
    void generateRookMoves(const Board& board, vector<Move>& moveList, Color currPlayer) {
        generateSlidingMoves(board, ROOK, STRAIGHT_DIRS, moveList, currPlayer);
    }
    void generateQueenMoves(const Board& board, vector<Move>& moveList, Color currPlayer) {
        generateSlidingMoves(board, QUEEN, DIAG_DIRS, moveList, currPlayer);
        generateSlidingMoves(board, QUEEN, STRAIGHT_DIRS, moveList, currPlayer);
    }
    void generateKingMoves(const Board& board, vector<Move>& moveList, Color currPlayer) {
        U64 kingMask = board.pieces[currPlayer][KING];

        int fromSquare = getLSB(kingMask);

        // loop through all normal king moves from curr square
        U64 kingAttack = kingAttacks[fromSquare] & ~board.occupancy[currPlayer];
        while (kingAttack) {
            int toSquare = getLSB(kingAttack);
            Piece captured = board.getPieceAt(currPlayer == WHITE ? BLACK : WHITE, toSquare);
            Move move(fromSquare, toSquare, KING, captured);
            moveList.push_back(move);    

            kingAttack &= kingAttack - 1;
        }

        // castling moves

        // white castling
        if (currPlayer == WHITE) {
            // kingside castling
            if (board.castlingRights & 0b1000) {
                if ((~board.occupancy[ALL] & 0x0000000000000060) == 0x0000000000000060) {
                    Move move(fromSquare, fromSquare + 2, KING, NONE, NONE, true);
                    moveList.push_back(move);
                }
            }
            // queenside castling
            if (board.castlingRights & 0b0100) {
                if ((~board.occupancy[ALL] & 0x000000000000000E) == 0x000000000000000E) {
                    Move move(fromSquare, fromSquare - 2, KING, NONE, NONE, true);
                    moveList.push_back(move);
                }
            }
        }
        // black castling
        else {
            // kingside castling
            if (board.castlingRights & 0b0010) {
                if ((~board.occupancy[ALL] & 0x6000000000000000) == 0x6000000000000000) {
                    Move move(fromSquare, fromSquare + 2, KING, NONE, NONE, true);
                    moveList.push_back(move);
                }
            }
            // queenside castling
            if (board.castlingRights & 0b0001) {
                if ((~board.occupancy[ALL] & 0x0E00000000000000) == 0x0E00000000000000) {
                    Move move(fromSquare, fromSquare - 2, KING, NONE, NONE, true);
                    moveList.push_back(move);
                }
            }
        }
    }
}

namespace MoveGen {

    void init() {
        preComputeKnightMoves();
        preComputeKingMoves();
    }

    U64 getRays(int fromSquare, U64 currPlayerPieces, U64 opponentPieces, const Direction* dirs) {
        U64 attacks = 0;
        
        for (int i = 0; i < 4; i++) {
            U64 currAttack = 1ULL << fromSquare;
            currAttack = dirs[i].shift > 0 ? currAttack << dirs[i].shift : currAttack >> -dirs[i].shift;
            while (currAttack & dirs[i].noWrap) {
                
                if (currAttack & currPlayerPieces) break;  // own piece, stop without adding
                attacks |= currAttack;
                if (currAttack & opponentPieces) break;    // enemy piece, add then stop
                currAttack = dirs[i].shift > 0 ? currAttack << dirs[i].shift : currAttack >> -dirs[i].shift;
            }
        }

        return attacks;
    }

    vector<Move> generateAllMoves(Color currPlayer, Board& board) {
        vector<Move> moveList;
        moveList.reserve(255);
        generatePawnMoves(board, moveList, currPlayer);
        generateKnightMoves(board, moveList, currPlayer);
        generateBishopMoves(board, moveList, currPlayer);
        generateRookMoves(board, moveList, currPlayer);
        generateQueenMoves(board, moveList, currPlayer);
        generateKingMoves(board, moveList, currPlayer);

        return moveList;
    }

    vector<Move> generateLegalMoves(Board& board) {
        Color currPlayer = board.sideToMove;
        vector<Move> moves = generateAllMoves(currPlayer, board);
        int kingIndex = getLSB(board.pieces[board.sideToMove][KING]);
        Color enemyColor = currPlayer == WHITE ? BLACK : WHITE;

        // check if king is currently in check
        bool canCastle = true;
        if (board.isSquareAttacked(enemyColor, kingIndex)) canCastle = false;
        
        vector<Move> legalMoves;
        legalMoves.reserve(moves.size());
        for (const Move& move : moves) {
            MoveInfo moveInfo = board.makeMove(move);
            if (!board.isKingInCheck()) {
                if (move.isCastle && canCastle) {
                    bool passesThroughCheck;
                    if (move.fromSquare < move.toSquare) {
                        passesThroughCheck = board.isSquareAttacked(enemyColor, move.toSquare - 1);
                    }
                    else {
                        passesThroughCheck = board.isSquareAttacked(enemyColor, move.toSquare + 1);
                    }
                    if (!board.isKingInCheck() && !passesThroughCheck) {
                        legalMoves.push_back(move);
                    }
                }
                else if (!move.isCastle) legalMoves.push_back(move);
            } 
            board.unMakeMove(moveInfo);    
        }

        return legalMoves;
    }
}