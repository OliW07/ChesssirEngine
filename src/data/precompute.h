#ifndef PRECOMPUTE_H
#define PRECOMPUTE_H

#include <cstdint>
#include <string>
#include <map>
#include <unordered_map>

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];
extern uint64_t whitePawnAttacks[64];
extern uint64_t blackPawnAttacks[64];
extern std::unordered_map<std::string, uint64_t[64]> Rays;

extern const std::map<std::string,int> Compass;
       

void precomputeBitBoardMoves();



#endif