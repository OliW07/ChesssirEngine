#ifndef PRECOMPUTE_H
#define PRECOMPUTE_H

#include <cstdint>
#include <string>
#include <map>

#include "utils/Types.h"

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];
extern uint64_t whitePawnAttacks[64];
extern uint64_t blackPawnAttacks[64];
extern uint64_t Rays[8][64];
extern const uint64_t RankMasks[8];

extern const std::map<RaysDirection,int> Compass;
       

void precomputeBitBoardMoves();



#endif
