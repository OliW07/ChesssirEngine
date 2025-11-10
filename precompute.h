#ifndef PRECOMPUTE_H
#define PRECOMPUTE_H

#include <cstdint>

extern const uint64_t KnightAttacks[64];
extern const uint64_t KingAttacks[64];

void precomputeBitBoardMoves();

#endif