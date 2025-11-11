#ifndef PRECOMPUTE_H
#define PRECOMPUTE_H

#include <cstdint>
#include <string>

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];

void precomputeBitBoardMoves();

int convertNotationToInt(const std::string &notation);
int convertLocationToRows(const int location);
int convertLocationToColumns(const int location);

#endif