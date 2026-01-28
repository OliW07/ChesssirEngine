#ifndef PRECOMPUTE_H
#define PRECOMPUTE_H

#include <cstdint>
#include <string>
#include <map>

#include "utils/Types.h"
#include "evaluate.h"


namespace precomputedData{

    extern uint64_t knightMoves[64];
    extern uint64_t kingMoves[64];
    extern uint64_t queenMoves[64];
    extern uint64_t rookMoves[64];
    extern uint64_t bishopMoves[64];
    extern uint64_t whitePawnMoves[64];
    extern uint64_t blackPawnMoves[64];
    extern uint64_t whitePawnAttacks[64];
    extern uint64_t blackPawnAttacks[64];
    extern uint64_t rays[8][64];
    extern uint64_t rankMasks[8];

    extern std::map<RaysDirection,int> compass;

    constexpr std::array<int, 64> pawnPieceSquares = {
        0,   0,   0,   0,   0,   0,   0,   0,
        5,  10,  10, -20, -20,  10,  10,   5,
        5,  -5, -10,   0,   0, -10,  -5,   5,
        0,   0,   0,  20,  20,   0,   0,   0,
        5,   5,  10,  25,  25,  10,   5,   5,
        10,  10,  20,  30,  30,  20,  10,  10,
        50,  50,  50,  50,  50,  50,  50,  50,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    constexpr std::array<int, 64> knightPieceSquares = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    constexpr std::array<int, 64> bishopPieceSquares = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    constexpr std::array<int, 64> rookPieceSquares = {
        0,   0,   0,   5,   5,   0,   0,   0,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
        5,  10,  10,  10,  10,  10,  10,   5,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    constexpr std::array<int, 64> queenPieceSquares = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -10,   5,   5,   5,   5,   5,   0, -10,
          0,   0,   5,   5,   5,   5,   0,  -5,
         -5,   0,   5,   5,   5,   5,   0,  -5,
        -10,   0,   5,   5,   5,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };

    constexpr std::array<int, 64> kingMiddlePieceSquares = {
         20,  30,  10,   0,   0,  10,  30,  20,
         20,  20,   0,   0,   0,   0,  20,  20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30
    };

    constexpr std::array<int, 64> kingEndPieceSquares = {
        -50, -30, -30, -30, -30, -30, -30, -50,
        -30, -30,   0,   0,   0,   0, -30, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -10,  30,  40,  40,  30, -10, -30,
        -30, -10,  30,  40,  40,  30, -10, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -20, -10,   0,   0, -10, -20, -30,
        -50, -40, -30, -20, -20, -30, -40, -50
    };




}

       
constexpr int mirrorSquare(int square) {
    return square ^ 56;
}




void precomputeBitBoardMoves();

inline int evaluatePieceSquare(int pieceType, int square, bool isWhite, bool isEndgame = false) {
    int value = 0;
    int tableSquare = isWhite ? square : mirrorSquare(square);
    
    switch(pieceType) {
        case Pawn:
            value = precomputedData::pawnPieceSquares[tableSquare];
            break;
        case Bishop:
            value =  precomputedData::bishopPieceSquares[tableSquare];
            break;
        case Knight:
            value = precomputedData::knightPieceSquares[tableSquare];
            break;
        case Rook:
            value = precomputedData::rookPieceSquares[tableSquare];
            break;
        case Queen:
            value = precomputedData::queenPieceSquares[tableSquare];
            break;
        case King:
            value = isEndgame ? precomputedData::kingEndPieceSquares[tableSquare] : precomputedData::kingMiddlePieceSquares[tableSquare];
            break;
        case None:
            value = 0;

    }
    
    return isWhite ? value : -value;
}

#endif
