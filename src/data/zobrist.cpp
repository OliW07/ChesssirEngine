#include "zobrist.h"

#include <random>

#include "board.h"

const ZobristKeys& getZobristKeys() {
    static const ZobristKeys keys = [] {
        ZobristKeys k;
        std::mt19937_64 rng(123456789);

        for (int piece = 0; piece < 6; piece++) {
            for (int square = 0; square < 64; square++) {
                k.pieceKeys[(size_t)Colour::White][piece][square] = rng();
                k.pieceKeys[(size_t)Colour::Black][piece][square] = rng();
            }
        }

        for (int castlingRight = 0; castlingRight < 16; castlingRight++) {
            k.castlingKeys[castlingRight] = rng();
        }

        for (int file = 0; file < 8; file++) {
            k.enPassantKeys[file] = rng();
        }

        k.sideKey = rng();

        return k;
    }();

    return keys;
}

uint64_t generateFullHash(Board& board) {
    uint64_t hash = 0ULL;

    for (int colour = 0; colour <= 1; ++colour) {
        for (int i = 0; i < board.state.pieceList.pieceCount[colour]; i++) {
            int loc = board.state.pieceList.list[colour][i];
            Pieces pieceType = (Pieces)(board.state.mailBox[loc] & ~8);

            hash ^= getZobristKeys().pieceKeys[colour][pieceType][loc];
        }
    }

    hash ^= getZobristKeys().castlingKeys[board.state.castlingRights];

    if (board.state.enPassantSquare != -1)
        hash ^= getZobristKeys().enPassantKeys[convertLocationToColumns(board.state.enPassantSquare)];

    if (!board.state.whiteToMove)
        hash ^= getZobristKeys().sideKey;

    return hash;
}
