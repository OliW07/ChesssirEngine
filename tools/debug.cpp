#include <iostream>
#include <bitset>
#include <cstdint>
#include <algorithm>
#include "utils/Types.h"
#include "debug.h"


void visualiseBitBoard(const uint64_t &bitBoard){
    std::bitset<64> boardBitSet(bitBoard);
    std::string bitString = boardBitSet.to_string();

    std::cout << "\n";

    for(int i = 0; i < 8; i++){
        std::string ss = bitString.substr(8*i, 8);

        std::reverse(ss.begin(),ss.end());

        for(char c : ss){
           std::cout << c << " ";
        }

        std::cout << "\n";
    }

    std::cout << "\n";
}

void visualiseGraphicBoard(BoardState state){

    char temp;

    std::cout << "\n";

    for(int rows = 7; rows >= 0; rows--){
        for(int columns = 0; columns < 8; columns++){

            int pos = 8*rows+columns;
        
            //At each location, check all possible piece and if present add to array

            bool whitePawn = state.whitePawnBitBoard & (1ULL << pos),
                whiteBishop = state.whiteBishopBitBoard & (1ULL << pos),
                whiteKnight = state.whiteKnightBitBoard & (1ULL << pos),
                whiteRook = state.whiteRookBitBoard & (1ULL << pos),
                whiteKing = state.whiteKingBitBoard & (1ULL << pos),
                whiteQueen = state.whiteQueenBitBoard & (1ULL << pos),
                blackPawn = state.blackPawnBitBoard & (1ULL << pos),
                blackBishop = state.blackBishopBitBoard & (1ULL << pos),
                blackKnight = state.blackKnightBitBoard & (1ULL << pos),
                blackRook = state.blackRookBitBoard & (1ULL << pos),
                blackKing = state.blackKingBitBoard & (1ULL << pos),
                blackQueen = state.blackQueenBitBoard & (1ULL << pos);

            /*state.whitePawnBitBoard <<= 1;
            state.whiteBishopBitBoard <<= 1;
            state.whiteKnightBitBoard <<= 1;
            state.whiteRookBitBoard <<= 1;
            state.whiteKingBitBoard <<= 1;
            state.whiteQueenBitBoard <<= 1;
            state.blackPawnBitBoard <<= 1;
            state.blackKnightBitBoard <<= 1;
            state.blackBishopBitBoard <<= 1;
            state.blackRookBitBoard <<= 1;
            state.blackKingBitBoard <<= 1;
            state.blackQueenBitBoard <<= 1;*/

            if(whitePawn) temp = 'P';
            else if(whiteBishop) temp = 'B';
            else if(whiteKnight) temp = 'N';
            else if(whiteRook) temp = 'R';
            else if(whiteQueen) temp = 'Q';
            else if(whiteKing) temp = 'K';
            else if(blackPawn) temp = 'p';
            else if(blackKnight) temp = 'n';
            else if(blackBishop) temp = 'b';
            else if(blackQueen) temp = 'q';
            else if(blackKing) temp = 'k';
            else if(blackRook) temp = 'r';
            else temp = '-';

            std::cout << temp << "   ";
        }

        std::cout << "\n\n";

    }

    std::cout << "\n";
}