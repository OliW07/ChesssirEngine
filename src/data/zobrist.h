#pragma once

#include <cstdint>

class Board;

struct ZobristKeys {
    uint64_t pieceKeys[2][6][64] = {0ULL};
    uint64_t castlingKeys[16] = {0ULL};
    uint64_t enPassantKeys[9] = {0ULL};
    uint64_t sideKey = {0ULL};
};

void initZobristKeys();

uint64_t generateFullHash(Board& board);
