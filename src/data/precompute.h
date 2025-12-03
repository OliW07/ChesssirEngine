#ifndef PRECOMPUTE_H
#define PRECOMPUTE_H

#include <cstdint>
#include <string>
#include <map>

#include "utils/Types.h"


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
}

       

void precomputeBitBoardMoves();



#endif
