#include "zobrist.h"
#include <random>
#include "board.h"
#include "Types.h"

ZobristKeys Zobrist;

void initZobristKeys(){
    std::mt19937_64 rng(123456789);

    for(int piece = 0; piece < 6; piece++){
        
        for(int square = 0; square < 64; square++){

            Zobrist.pieceKeys[White][piece][square] = rng();
            Zobrist.pieceKeys[Black][piece][square] = rng();
        }
    }

    for(int castlingRight = 0; castlingRight < 16; castlingRight++){
        Zobrist.castlingKeys[castlingRight] = rng();
    }

    for(int file = 0; file < 8; file++){
        Zobrist.enPassantKeys[file] = rng();
    } 

    Zobrist.sideKey = rng();
}

uint64_t generateFullHash(Board &board){
    uint64_t hash = 0ULL;

    for(int Colour = Black; Colour < White; Colour++){

        for(int i = 0; i < board.state.pieceList.pieceCount[Colour]; i++){
            int loc = board.state.pieceList.list[Colour][i];
            Pieces pieceType = (Pieces)(board.state.mailBox[loc] &= ~8);
            
            hash ^= Zobrist.pieceKeys[Colour][pieceType][loc];


        }   
    }

    hash ^= Zobrist.castlingKeys[board.state.castlingRights];
   
    if(board.state.enPassantSquare != -1) hash ^= Zobrist.enPassantKeys[convertLocationToColumns(board.state.enPassantSquare)];

    if(!board.state.whiteToMove) hash ^= Zobrist.sideKey;

    return hash;
}
