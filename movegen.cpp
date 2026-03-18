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

// precompute king and knight pieces attack tables
namespace {
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
            
            MoveGen::knightAttacks[i] = currMoveMap;
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
            
            MoveGen::kingAttacks[i] = currMoveMap;
        }
    }
}

// Pawn and sliding pieces move functions
namespace {
    void wPawnSinglePush(const Board& board, Move* moveList, int& moveCount) {
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
            moveList[moveCount] = move; // add to move list
            moveCount++;
            
            singlePush &= singlePush - 1; // move onto next least significant bit
        }
        
        // all promo moves possible
        while (promoPawns) {
            int toSquare = getLSB(promoPawns);
            int fromSquare = toSquare - 8;
            
            // All 4 promotions
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, QUEEN));
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, ROOK)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, BISHOP)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, KNIGHT)); 
            moveCount++; 
            
            promoPawns &= promoPawns - 1;
        }
    }
    void wPawnDoublePush(const Board& board, Move* moveList, int& moveCount) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank2Pawns = board.pieces[WHITE][PAWN] & 0x000000000000FF00ULL;
        U64 singlePush = (rank2Pawns << 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush << 8) & emptySquares;       // destination must also be empty

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare - 16;
            Move move(fromSquare, toSquare, PAWN);
            moveList[moveCount] = move; // add to move list
            moveCount++;

            doublePush &= doublePush - 1; // move onto next least significant bit
        }
    }
    void wPawnCapture(const Board& board, Move* moveList, int& moveCount) {
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
            moveList[moveCount] = move; // add to move list
            moveCount++;

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare - 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList[moveCount] = move; // add to move list
            moveCount++;

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
        }

        // Check left promotion captures
        while (leftPromoCaptures) {
            int toSquare = getLSB(leftPromoCaptures);
            int fromSquare = toSquare - 7;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);

            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, ROOK)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, BISHOP)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, KNIGHT)); 
            moveCount++; 
            
            leftPromoCaptures &= leftPromoCaptures - 1; // move onto next least significant bit
        }
        
        // Check right promotion captures
        while (rightPromoCaptures) {
            int toSquare = getLSB(rightPromoCaptures);
            int fromSquare = toSquare - 9;
            
            Piece captured = board.getPieceAt(BLACK, toSquare);

            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, ROOK)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, BISHOP)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, KNIGHT)); 
            moveCount++; 

            rightPromoCaptures &= rightPromoCaptures - 1; // move onto next least significant bit
        }

        // check for en passant moves
        if (board.enPassantSquare != -1) {   
            if ((board.pieces[WHITE][PAWN] & ~MASK_A_FILE) << 7 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare - 7, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList[moveCount] = move; // add to move list
                moveCount++;
            }
            
            if ((board.pieces[WHITE][PAWN] & ~MASK_H_FILE) << 9 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare - 9, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList[moveCount] = move; // add to move list
                moveCount++;
            }
        }
    }
    void bPawnSinglePush(const Board& board, Move* moveList, int& moveCount) {
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
            moveList[moveCount] = move; // add to move list
            moveCount++;

            singlePush &= singlePush - 1; // move onto next least significant bit
        }

        while (promoPawns) {
            int toSquare = getLSB(promoPawns);
            int fromSquare = toSquare + 8;
            
            // All 4 promotions
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, QUEEN));
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, ROOK)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, BISHOP)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, NONE, KNIGHT)); 
            moveCount++; 

            promoPawns &= promoPawns - 1;
        }
    }
    void bPawnDoublePush(const Board& board, Move* moveList, int& moveCount) {
        U64 emptySquares = ~board.occupancy[ALL];
        U64 rank7Pawns = board.pieces[BLACK][PAWN] & 0x00FF000000000000ULL;
        U64 singlePush = (rank7Pawns >> 8) & emptySquares;      // intermediate square must be empty
        U64 doublePush = (singlePush >> 8) & emptySquares;       // destination must also be empty

        // Find all 1 bits to generate moves
        while (doublePush) {
            int toSquare = getLSB(doublePush);
            int fromSquare = toSquare + 16;
            Move move(fromSquare, toSquare, PAWN);
            moveList[moveCount] = move; // add to move list
            moveCount++;

            doublePush &= doublePush - 1; // move onto next least significant bit
        }
    }
    void bPawnCapture(const Board& board, Move* moveList, int& moveCount) {
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
            moveList[moveCount] = move; // add to move list
            moveCount++;

            leftCaptures &= leftCaptures - 1; // move onto next least significant bit
        }
        
        // Check right captures
        while (rightCaptures) {
            int toSquare = getLSB(rightCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            Move move(fromSquare, toSquare, PAWN, captured);
            moveList[moveCount] = move; // add to move list
            moveCount++;

            rightCaptures &= rightCaptures - 1; // move onto next least significant bit
        }

        // Check left promotion captures
        while (leftPromoCaptures) {
            int toSquare = getLSB(leftPromoCaptures);
            int fromSquare = toSquare + 7;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);
            
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, ROOK)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, BISHOP)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, KNIGHT)); 
            moveCount++; 
            
            leftPromoCaptures &= leftPromoCaptures - 1; // move onto next least significant bit
        }
        
        // Check right promotion captures
        while (rightPromoCaptures) {
            int toSquare = getLSB(rightPromoCaptures);
            int fromSquare = toSquare + 9;
            
            Piece captured = board.getPieceAt(WHITE, toSquare);

            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, QUEEN));
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, ROOK)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, BISHOP)); 
            moveCount++;
            moveList[moveCount] = (Move(fromSquare, toSquare, PAWN, captured, KNIGHT)); 
            moveCount++; 

            rightPromoCaptures &= rightPromoCaptures - 1; // move onto next least significant bit
        }

        // check for en passant moves
        if (board.enPassantSquare != -1) {   
            if ((board.pieces[BLACK][PAWN] & ~MASK_H_FILE) >> 7 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare + 7, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList[moveCount] = move; // add to move list
                moveCount++;
            }
            
            if ((board.pieces[BLACK][PAWN] & ~MASK_A_FILE) >> 9 & (1ULL << board.enPassantSquare)) {
                Move move(board.enPassantSquare + 9, board.enPassantSquare, PAWN, PAWN, NONE, false, true);
                moveList[moveCount] = move; // add to move list
                moveCount++;
            }
        }
    }
    void generateSlidingMoves(const Board& board, Piece piece, const Direction* dirs, Move* moveList, int& moveCount, Color currPlayer) {
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
                moveList[moveCount] = move; // add to move list
                moveCount++;

                currAttacks &= currAttacks - 1;
            }

            currColorPieces &= currColorPieces - 1;
        }
    }
}

// move generation function for each individual type of piece
namespace {
    void generatePawnMoves(const Board& board, Move* moveList, int& moveCount, Color currPlayer) {

        if (currPlayer == WHITE) {
            wPawnSinglePush(board, moveList, moveCount); 
            wPawnDoublePush(board, moveList, moveCount);
            wPawnCapture(board, moveList, moveCount);
        }
        else {
            bPawnSinglePush(board, moveList, moveCount);
            bPawnDoublePush(board, moveList, moveCount); 
            bPawnCapture(board, moveList, moveCount);  
        }
    }
    void generateKnightMoves(const Board& board, Move* moveList, int& moveCount, Color currPlayer) {
        U64 knights = board.pieces[currPlayer][KNIGHT];

        // Go through all the players knights
        while (knights) {
            int fromSquare = getLSB(knights);
            
            // Retrieve knight attack of each player knight and make sure it cannot capture own player pieces
            U64 knightMoves = MoveGen::knightAttacks[fromSquare] & ~board.occupancy[currPlayer];
            // Loop through all moves retrieved and store it
            while (knightMoves) {
                int toSquare = getLSB(knightMoves);
                Piece captured = board.getPieceAt(currPlayer == WHITE ? BLACK : WHITE, toSquare);
                Move move(fromSquare, toSquare, KNIGHT, captured); // Captured == None if no enemy piece exists at toSquare
                moveList[moveCount] = move; // add to move list
                moveCount++;
                knightMoves &= knightMoves - 1;
            }

            knights &= knights - 1;
        }
    }
    void generateBishopMoves(const Board& board, Move* moveList, int& moveCount, Color currPlayer) {
        generateSlidingMoves(board, BISHOP, DIAG_DIRS, moveList, moveCount, currPlayer);
    }
    void generateRookMoves(const Board& board, Move* moveList, int& moveCount, Color currPlayer) {
        generateSlidingMoves(board, ROOK, STRAIGHT_DIRS, moveList, moveCount, currPlayer);
    }
    void generateQueenMoves(const Board& board, Move* moveList, int& moveCount, Color currPlayer) {
        generateSlidingMoves(board, QUEEN, DIAG_DIRS, moveList, moveCount, currPlayer);
        generateSlidingMoves(board, QUEEN, STRAIGHT_DIRS, moveList, moveCount, currPlayer);
    }
    void generateKingMoves(const Board& board, Move* moveList, int& moveCount, Color currPlayer) {
        U64 kingMask = board.pieces[currPlayer][KING];

        int fromSquare = getLSB(kingMask);

        // loop through all normal king moves from curr square
        U64 kingAttack = MoveGen::kingAttacks[fromSquare] & ~board.occupancy[currPlayer];
        while (kingAttack) {
            int toSquare = getLSB(kingAttack);
            Piece captured = board.getPieceAt(currPlayer == WHITE ? BLACK : WHITE, toSquare);
            Move move(fromSquare, toSquare, KING, captured);
            moveList[moveCount] = move; // add to move list
            moveCount++;  

            kingAttack &= kingAttack - 1;
        }

        // castling moves

        // white castling
        if (currPlayer == WHITE) {
            // kingside castling
            if (board.castlingRights & 0b1000) {
                if ((~board.occupancy[ALL] & 0x0000000000000060) == 0x0000000000000060) {
                    Move move(fromSquare, fromSquare + 2, KING, NONE, NONE, true);
                    moveList[moveCount] = move; // add to move list
                    moveCount++;
                }
            }
            // queenside castling
            if (board.castlingRights & 0b0100) {
                if ((~board.occupancy[ALL] & 0x000000000000000E) == 0x000000000000000E) {
                    Move move(fromSquare, fromSquare - 2, KING, NONE, NONE, true);
                    moveList[moveCount] = move; // add to move list
                    moveCount++;
                }
            }
        }
        // black castling
        else {
            // kingside castling
            if (board.castlingRights & 0b0010) {
                if ((~board.occupancy[ALL] & 0x6000000000000000) == 0x6000000000000000) {
                    Move move(fromSquare, fromSquare + 2, KING, NONE, NONE, true);
                    moveList[moveCount++] = move; // add to move list
                }
            }
            // queenside castling
            if (board.castlingRights & 0b0001) {
                if ((~board.occupancy[ALL] & 0x0E00000000000000) == 0x0E00000000000000) {
                    Move move(fromSquare, fromSquare - 2, KING, NONE, NONE, true);
                    moveList[moveCount++] = move; // add to move list
                }
            }
        }
    }
}

namespace MoveGen {
    U64 knightAttacks[64];
    U64 kingAttacks[64];

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

    void generateAllMoves(Color currPlayer, Move* moveList, int& moveCount, Board& board) {

        generatePawnMoves(board, moveList, moveCount, currPlayer);
        generateKnightMoves(board, moveList, moveCount, currPlayer);
        generateBishopMoves(board, moveList, moveCount, currPlayer);
        generateRookMoves(board, moveList, moveCount, currPlayer);
        generateQueenMoves(board, moveList, moveCount, currPlayer);
        generateKingMoves(board, moveList, moveCount, currPlayer);
    }

    void generateLegalMoves(Board& board, Move* legalMoves, int& legalMovesCount) {
        Move allMoves[256];
        int moveCount = 0;
        Color currPlayer = board.sideToMove;
        generateAllMoves(currPlayer, allMoves, moveCount, board);
        int kingIndex = getLSB(board.pieces[board.sideToMove][KING]);
        Color enemyColor = currPlayer == WHITE ? BLACK : WHITE;

        // check if king is currently in check
        bool canCastle = true;
        if (board.isSquareAttacked(enemyColor, kingIndex)) canCastle = false;
        
        for (int i = 0; i < moveCount; i++) {
            Move& move = allMoves[i];
            MoveInfo moveInfo = board.makeMove(move);
            if (!board.isKingInCheck(currPlayer)) {
                if (move.isCastle && canCastle) {
                    bool passesThroughCheck;
                    if (move.fromSquare < move.toSquare) 
                        passesThroughCheck = board.isSquareAttacked(enemyColor, move.toSquare - 1);
                    else 
                        passesThroughCheck = board.isSquareAttacked(enemyColor, move.toSquare + 1);
                    if (!passesThroughCheck) 
                        legalMoves[legalMovesCount++] = move;
                }
                else if (!move.isCastle) 
                    legalMoves[legalMovesCount++] = move;
            } 
            board.unMakeMove(moveInfo);    
        }
    }
}