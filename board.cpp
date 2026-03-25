#include <iostream>
#include <cmath>
#include <random>
#include <sstream>
#include <string>
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "moveinfo.h"
using namespace std;

namespace {
    void updateOccupancyBoards(Board& board) {
        board.occupancy[WHITE] = board.pieces[WHITE][ROOK] | board.pieces[WHITE][KNIGHT]
                                | board.pieces[WHITE][BISHOP] | board.pieces[WHITE][KING]
                                | board.pieces[WHITE][QUEEN] | board.pieces[WHITE][PAWN];

        board.occupancy[BLACK] = board.pieces[BLACK][ROOK] | board.pieces[BLACK][KNIGHT]
                                | board.pieces[BLACK][BISHOP] | board.pieces[BLACK][KING]
                                | board.pieces[BLACK][QUEEN] | board.pieces[BLACK][PAWN];

        board.occupancy[ALL] = board.occupancy[WHITE] | board.occupancy[BLACK];
    }
}

// namespace for zobristhashing initilization
namespace {
    bool initialized = false;

    U64 zobristTable[2][6][64];
    U64 zobristWhiteToMove;
    U64 zobristCastling[16];
    U64 zobristEnPassant[8];

    void initZobrist() {
        std::mt19937_64 rng(301005); // any fixed seed
        std::uniform_int_distribution<uint64_t> dist;

        for (int c = 0; c < 2; c++)
            for (int p = 0; p < 6; p++)
                for (int s = 0; s < 64; s++)
                    zobristTable[c][p][s] = dist(rng);

        zobristWhiteToMove = dist(rng);

        for (int i = 0; i < 16; i++)
            zobristCastling[i] = dist(rng);

        for (int i = 0; i < 8; i++)
            zobristEnPassant[i] = dist(rng);
        
        initialized = true;
    }
}

void Board::loadFEN(string FEN) {
    // zero everything out first
    memset(pieces, 0, sizeof(pieces));
    memset(occupancy, 0, sizeof(occupancy));

    vector<string> tokens;
    string token;
    istringstream ss(FEN);
    while (getline(ss, token, ' ')) {
        tokens.push_back(token);
    }

    int currSquare = 56;

    // load chess pieces
    for (char c : tokens[0]) {
        if (isdigit(static_cast<unsigned char>(c))) {
            currSquare += (c - '0');
        }
        else if (c == '/') currSquare -= 16;
        else {
            if (c >= 'a' && c <= 'z') {
                if (c == 'r') {
                    pieces[BLACK][ROOK] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'n') {
                    pieces[BLACK][KNIGHT] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'b') {
                    pieces[BLACK][BISHOP] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'q') {
                    pieces[BLACK][QUEEN] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'k') {
                    pieces[BLACK][KING] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'p') {
                    pieces[BLACK][PAWN] ^= (1ULL << currSquare);
                    currSquare++;
                }
            }
            else {
                if (c == 'R') {
                    pieces[WHITE][ROOK] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'N') {
                    pieces[WHITE][KNIGHT] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'B') {
                    pieces[WHITE][BISHOP] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'Q') {
                    pieces[WHITE][QUEEN] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'K') {
                    pieces[WHITE][KING] ^= (1ULL << currSquare);
                    currSquare++;
                }
                else if (c == 'P') {
                    pieces[WHITE][PAWN] ^= (1ULL << currSquare);
                    currSquare++;
                }
            }
        }
    }

    updateOccupancyBoards(*this);

    // set side to move
    sideToMove = tokens[1] == "w" ? WHITE : BLACK;

    // sets castling
    castlingRights = 0b0000;
    for (char c : tokens[2]) {
        if (c == 'K') castlingRights ^= 0b1000;
        else if (c == 'Q') castlingRights ^= 0b0100;
        else if (c == 'k') castlingRights ^= 0b0010;
        else if (c == 'q') castlingRights ^= 0b0001;
    }

    // sets en-passant square
    if (tokens[3] != "-") {
        int file = tokens[3][0] - 'a';
        int rank = tokens[3][1] - '1';
        enPassantSquare = rank * 8 + file;
    }
    else enPassantSquare = -1;

    // set half move clock
    halfMoveClock = stoi(tokens[4]);

    // set full move number
    fullMoveNumber = stoi(tokens[5]);
}

void Board::init(string FEN) {
    loadFEN(FEN);

    // pre-compute knight moves
    MoveGen::init();
    if (!initialized) initZobrist();
    hash = 0;
    historyPly = 0;
}

void Board::printBoard() {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file <= 7; file++) {
            int sq = rank * 8 + file;
            U64 mask = 1ULL << sq;

            char currPiece = '.';

            if (pieces[WHITE][ROOK] & mask) currPiece = 'R';
            else if (pieces[WHITE][KNIGHT] & mask) currPiece = 'N';
            else if (pieces[WHITE][BISHOP] & mask) currPiece = 'B';
            else if (pieces[WHITE][QUEEN] & mask) currPiece = 'Q';
            else if (pieces[WHITE][KING] & mask) currPiece = 'K';
            else if (pieces[WHITE][PAWN] & mask) currPiece = 'P';
            
            else if (pieces[BLACK][ROOK] & mask) currPiece = 'r';
            else if (pieces[BLACK][KNIGHT] & mask) currPiece = 'n';
            else if (pieces[BLACK][BISHOP] & mask) currPiece = 'b';
            else if (pieces[BLACK][QUEEN] & mask) currPiece = 'q';
            else if (pieces[BLACK][KING] & mask) currPiece = 'k';
            else if (pieces[BLACK][PAWN] & mask) currPiece = 'p';

            cout << currPiece;
        }
        cout << endl;
    }
}

Piece Board::getPieceAt(Color color, int square) const {
    U64 mask = 1ULL << square;
    for (int pt = KING; pt <= PAWN; pt++) {
        if (pieces[color][pt] & mask)
            return static_cast<Piece>(pt);
    }
    return NONE;
}

MoveInfo Board::makeMove(Move move) {
    // hash xoring out is done for zobrist hashing all throughout this function
    MoveInfo moveInfo(move, enPassantSquare, castlingRights, halfMoveClock, fullMoveNumber, sideToMove);
    
    if (enPassantSquare != -1) hash ^= zobristEnPassant[enPassantSquare % 8];
    enPassantSquare = -1; // update enPassant
    
    // basic captures and en passant moves
    if (move.promotionPiece == NONE && !move.isCastle){
        pieces[sideToMove][move.pieceType] ^= (1ULL << move.fromSquare);
        hash ^= zobristTable[sideToMove][move.pieceType][move.fromSquare];
        pieces[sideToMove][move.pieceType] ^= (1ULL << move.toSquare);
        hash ^= zobristTable[sideToMove][move.pieceType][move.toSquare];
        
        // checks if move was double pawn push
        if (move.pieceType == PAWN && abs(move.fromSquare - move.toSquare) == 16) {
            enPassantSquare = sideToMove == WHITE ? move.toSquare - 8 : move.toSquare + 8;
            hash ^= zobristEnPassant[enPassantSquare % 8];
        }
        // special conditions if move was enpassant
        else if (move.capturedPiece == PAWN && move.isEnPassant) {
            if (sideToMove == WHITE) {
                pieces[BLACK][PAWN] ^= (1ULL << (move.toSquare - 8));
                hash ^= zobristTable[BLACK][PAWN][move.toSquare - 8];
            }
            if (sideToMove == BLACK) {
                pieces[WHITE][PAWN] ^= (1ULL << (move.toSquare + 8));
                hash ^= zobristTable[WHITE][PAWN][move.toSquare + 8];
            }
            halfMoveClock = 0;
        }
        // checks if normal capture took place
        else if (move.capturedPiece != NONE) {
            pieces[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece] ^= (1ULL << move.toSquare);
            hash ^= zobristTable[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece][move.toSquare];
            halfMoveClock = 0;

            // update castling rights if captured piece was rook
            if (move.capturedPiece == ROOK) {
                hash ^= zobristCastling[castlingRights];
                if (sideToMove == BLACK) {
                    if (move.toSquare == 0) {
                        if (castlingRights & 0b0100) castlingRights ^= 0b0100;
                    }
                    else if (move.toSquare == 7) {
                        if (castlingRights & 0b1000) castlingRights ^= 0b1000;
                    }
                }
                else {
                    if (move.toSquare == 56) {
                        if (castlingRights & 0b0001) castlingRights ^= 0b0001;
                    }
                    else if (move.toSquare == 63) {
                        if (castlingRights & 0b0010) castlingRights ^= 0b0010;
                    }
                }
                hash ^= zobristCastling[castlingRights];
            }
        }
        
        // update castling rights if necessary
        if (move.pieceType == ROOK) {
            hash ^= zobristCastling[castlingRights];
            if (sideToMove == WHITE) {
                if (move.fromSquare == 0) {
                    if (castlingRights & 0b0100) castlingRights ^= 0b0100;
                }
                else if (move.fromSquare == 7) {
                    if (castlingRights & 0b1000) castlingRights ^= 0b1000;
                }
            }
            else {
                if (move.fromSquare == 56) {
                    if (castlingRights & 0b0001) castlingRights ^= 0b0001;
                }
                else if (move.fromSquare == 63) {
                    if (castlingRights & 0b0010) castlingRights ^= 0b0010;
                }
            }
            hash ^= zobristCastling[castlingRights];
        }
        else if (move.pieceType == KING) {
            hash ^= zobristCastling[castlingRights];
            if (sideToMove == WHITE) castlingRights &= 0b0011;
            else castlingRights &= 0b1100;
            hash ^= zobristCastling[castlingRights];
        }
    }
    // promotion move
    else if (move.promotionPiece != NONE) {
        pieces[sideToMove][PAWN] ^= (1ULL << move.fromSquare); // removes pawn from bitboard
        hash ^= zobristTable[sideToMove][PAWN][move.fromSquare];
        pieces[sideToMove][move.promotionPiece] ^= (1ULL << move.toSquare); // adds promoted pieces to appropriate bitboard
        hash ^= zobristTable[sideToMove][move.promotionPiece][move.toSquare];
        
        // removes captured piece from appropriate bitboard
        if (move.capturedPiece != NONE) {
            pieces[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece] ^= (1ULL << move.toSquare);
            hash ^= zobristTable[sideToMove == WHITE ? BLACK : WHITE][move.capturedPiece][move.toSquare];
            halfMoveClock = 0;
            
            // update castling rights if captured piece was rook
            if (move.capturedPiece == ROOK) {
                hash ^= zobristCastling[castlingRights];
                if (sideToMove == BLACK) {
                    if (move.toSquare == 0) {
                        if (castlingRights & 0b0100) castlingRights ^= 0b0100;
                    }
                    else if (move.toSquare == 7) {
                        if (castlingRights & 0b1000) castlingRights ^= 0b1000;
                    }
                }
                else {
                    if (move.toSquare == 56) {
                        if (castlingRights & 0b0001) castlingRights ^= 0b0001;
                    }
                    else if (move.toSquare == 63) {
                        if (castlingRights & 0b0010) castlingRights ^= 0b0010;
                    }
                }
                hash ^= zobristCastling[castlingRights];
            }
        }
    }
    // castling moves
    else if (move.isCastle) {
        hash ^= zobristCastling[castlingRights];
        if (move.toSquare > move.fromSquare) {
            // move rook to correct spot
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare + 3);
            hash ^= zobristTable[sideToMove][ROOK][move.fromSquare + 3];
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare + 1);
            hash ^= zobristTable[sideToMove][ROOK][move.fromSquare + 1];
        }
        else {
            // move rook to correct spot
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare - 4);
            hash ^= zobristTable[sideToMove][ROOK][move.fromSquare - 4];
            pieces[sideToMove][ROOK] ^= (1ULL << move.fromSquare - 1);
            hash ^= zobristTable[sideToMove][ROOK][move.fromSquare - 1];
        }
        if (sideToMove == WHITE) castlingRights &= 0b0011;
        if (sideToMove == BLACK) castlingRights &= 0b1100;
        hash ^= zobristCastling[castlingRights];
        // move king to correct spot
        pieces[sideToMove][KING] ^= (1ULL << move.fromSquare);
        hash ^= zobristTable[sideToMove][KING][move.fromSquare];
        pieces[sideToMove][KING] ^= (1ULL << move.toSquare);
        hash ^= zobristTable[sideToMove][KING][move.toSquare];
    }

    // halfMoveClock update
    if (move.pieceType == PAWN || move.capturedPiece != NONE) halfMoveClock = 0;
    else halfMoveClock++;

    // fullMove update
    if (sideToMove == BLACK) fullMoveNumber++;

    sideToMove = sideToMove == WHITE ? BLACK : WHITE;

    hash ^= zobristWhiteToMove;

    // add current hash to hashHistory array
    hashHistory[historyPly++] = hash;

    updateOccupancyBoards(*this);
    return moveInfo;
}

void Board::unMakeMove(MoveInfo moveInfo) {
    Move move = moveInfo.move;

    // move piece back to previous standing place
    pieces[moveInfo.prevColor][move.pieceType] ^= (1ULL << move.toSquare);
    pieces[moveInfo.prevColor][move.pieceType] ^= (1ULL << move.fromSquare);

    // check if castle
    if (move.isCastle) {
        if (move.toSquare > move.fromSquare) {
            // move rook to back to original spot
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare + 1);
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare + 3);
        }
        else {
            // move rook to back to original spot
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare - 1);
            pieces[moveInfo.prevColor][ROOK] ^= (1ULL << move.fromSquare - 4);
        }
    }
    // check if promotion
    else if (move.promotionPiece != NONE) {
        pieces[moveInfo.prevColor][PAWN] ^= (1ULL << move.toSquare); // undo the accidental creation of pawn above in the function
        pieces[moveInfo.prevColor][move.promotionPiece] ^= (1ULL << move.toSquare); // remove promoted piece
    }
    else if (move.isEnPassant) {
        // if captured pawn was white
        if (sideToMove == WHITE) {
            pieces[WHITE][PAWN] ^= (1ULL << (moveInfo.prevEnPassantSquare + 8));
        }
        // if captured pawn was black
        else {
            pieces[BLACK][PAWN] ^= (1ULL << (moveInfo.prevEnPassantSquare - 8));
        }
    }
    // all normal captures
    if (move.capturedPiece != NONE && !move.isEnPassant) {
        pieces[sideToMove][move.capturedPiece] ^= (1ULL << move.toSquare);
    }

    // reset all variables to previous state
    sideToMove = moveInfo.prevColor;
    enPassantSquare = moveInfo.prevEnPassantSquare;
    castlingRights = moveInfo.prevCastling;
    halfMoveClock = moveInfo.prevHalfMoveClock;
    fullMoveNumber = moveInfo.prevFullMoveNumber;

    // update all occupancy boards
    updateOccupancyBoards(*this);

    // revert hash back to before this move
    if (historyPly - 2 >= 0) {
        hash = hashHistory[historyPly - 2];
        historyPly--;    
    }
    else hash = 0; // first move (initial hash)
}

bool Board::isKingInCheck(Color kingColor) {
    int kingIndex = getLSB(pieces[kingColor][KING]);
    return isSquareAttacked(kingColor == WHITE ? BLACK : WHITE, kingIndex);
} 

bool Board::isSquareAttacked(Color attackingPlayer, int toSquare) {
    Color currColor = attackingPlayer == WHITE ? BLACK : WHITE;
    // Knights
    if (MoveGen::knightAttacks[toSquare] & pieces[attackingPlayer][KNIGHT]) return true;
    // Kings
    if (MoveGen::kingAttacks[toSquare] & pieces[attackingPlayer][KING]) return true;
    // Pawns
    U64 sqMask = 1ULL << toSquare;
    if (attackingPlayer == WHITE) {
        U64 pawnAttacks = ((sqMask >> 7) & ~MASK_A_FILE) | ((sqMask >> 9) & ~MASK_H_FILE);
        if (pawnAttacks & pieces[attackingPlayer][PAWN]) return true;
    } else {
        U64 pawnAttacks = ((sqMask << 7) & ~MASK_H_FILE) | ((sqMask << 9) & ~MASK_A_FILE);
        if (pawnAttacks & pieces[attackingPlayer][PAWN]) return true;
    }
    // Bishops + Queens
    if (MoveGen::getRays(toSquare, occupancy[currColor], occupancy[attackingPlayer], DIAG_DIRS) &
       (pieces[attackingPlayer][BISHOP] | pieces[attackingPlayer][QUEEN])) return true;
    // Rooks + Queens
    if (MoveGen::getRays(toSquare, occupancy[currColor], occupancy[attackingPlayer], STRAIGHT_DIRS) &
       (pieces[attackingPlayer][ROOK] | pieces[attackingPlayer][QUEEN])) return true;
    return false;
}

bool Board::isInsufficientMaterial() {
    // check if queen, rook, or pawns exist for either side
    if (pieces[WHITE][QUEEN] | pieces[WHITE][ROOK] | pieces[WHITE][PAWN]
      | pieces[BLACK][QUEEN] | pieces[BLACK][ROOK] | pieces[BLACK][PAWN]) return false;
    
    // if made here, neither side has queens, rooks or pawns

    // check if only kings exist
    if (!(pieces[WHITE][KNIGHT] | pieces[BLACK][KNIGHT] 
      | pieces[WHITE][BISHOP] | pieces[BLACK][BISHOP])) return true;
    // above condition confirms that at least black or white has a knight or bishop
    
    // check if white only has king
    if (occupancy[WHITE] == pieces[WHITE][KING]) {
        // check if black only has 1 bishop
        U64 blackBishops = pieces[BLACK][BISHOP];
        int count = 0;
        while (blackBishops) {
            count++;
            blackBishops &= blackBishops - 1;
        }

        if (count == 1 && (pieces[BLACK][BISHOP] | pieces[BLACK][KING]) == occupancy[BLACK]) return true;
        
        // check if black only has 1 knight
        U64 blackKnights = pieces[BLACK][KNIGHT];
        count = 0;
        while (blackKnights) {
            count++;
            blackKnights &= blackKnights - 1;
        }

        if (count == 1 && (pieces[BLACK][KNIGHT] | pieces[BLACK][KING]) == occupancy[BLACK]) return true;
    }
    
    // check if black only has king
    if (occupancy[BLACK] == pieces[BLACK][KING]) {
        // check if black only has 1 bishop
        U64 whiteBishops = pieces[WHITE][BISHOP];
        int count = 0;
        while (whiteBishops) {
            count++;
            whiteBishops &= whiteBishops - 1;
        }

        if (count == 1 && (pieces[WHITE][BISHOP] | pieces[WHITE][KING]) == occupancy[WHITE]) return true;
        
        // check if black only has 1 knight
        U64 whiteKnights = pieces[WHITE][KNIGHT];
        count = 0;
        while (whiteKnights) {
            count++;
            whiteKnights &= whiteKnights - 1;
        }

        if (count == 1 && (pieces[WHITE][KNIGHT] | pieces[WHITE][KING]) == occupancy[WHITE]) return true;
    }
    
    // check if both sides only have king + bishop (same color)
    if (!(pieces[WHITE][KNIGHT] | pieces[BLACK][KNIGHT])) {
        U64 whiteBishops = pieces[WHITE][BISHOP];
        U64 blackBishops = pieces[BLACK][BISHOP];
        // count all white bishops
        int whiteBishopsCount = 0;
        while (whiteBishops) {
            whiteBishopsCount++;
            whiteBishops &= whiteBishops - 1;
        }
        
        // count all black bishops
        int blackBishopsCount = 0;
        while(blackBishops) {
            blackBishopsCount++;
            blackBishops &= blackBishops - 1;
        }

        // check if only one bishop exists for each player
        if (whiteBishopsCount == 1 && blackBishopsCount == 1) {
            // check if bishops are on the same colored square
            if (IS_WHITE[getLSB(pieces[WHITE][BISHOP])] == IS_WHITE[getLSB(pieces[BLACK][BISHOP])])
                return true;
        }
    }
    else {
        if (!(pieces[WHITE][BISHOP] | pieces[BLACK][BISHOP])) {
            U64 whiteKnights = pieces[WHITE][KNIGHT];
            U64 blackKnights = pieces[BLACK][KNIGHT];
            // count all white knights
            int whiteKnightsCount = 0;
            while (whiteKnights) {
                whiteKnightsCount++;
                whiteKnights &= whiteKnights - 1;
            }
            
            // count all black knights
            int blackKnightsCount = 0;
            while(blackKnights) {
                blackKnightsCount++;
                blackKnights &= blackKnights - 1;
            }

            // check if only one knight exists for each player
            if (whiteKnightsCount == 1 && blackKnightsCount == 1) 
                return true;
        }
    }

    return false;
}