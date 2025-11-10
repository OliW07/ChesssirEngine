#include "precompute.h"
#include <iostream>
#include <cmath>

uint64_t KnightMoves[64] = {};
uint64_t KingMoves[64] = {};
uint64_t QueenMoves[64] = {};
uint64_t RookMoves[64] = {};
uint64_t BishopMoves[64] = {};
uint64_t whitePawnMoves[64] = {};
uint64_t blackPawnMoves[64] = {};



void computeKnightMoves(const int pos);
void computeKingMoves(const int pos);
void computeQueenMoves(const int pos);
void computeRookMoves(const int pos);
void computeBishopMoves(const int pos);
void computePawnMoves(const int pos, bool isWhite);


uint64_t returnSlidingBitBoard(const int pos, bool diagonalSliding);


void precomputeBitBoardMoves(){

    std::cout << "Precomputing move data.. \n";
    
    for(int i = 0; i < 64; i++){
        computeKnightMoves(i);
        computeKingMoves(i);
        computeQueenMoves(i);
        computeRookMoves(i);
        computeBishopMoves(i);
        computePawnMoves(i,true);
        computePawnMoves(i,false);
    }

    
    std::cout << "Successfully precomputed bit board data \n";
}

void computeKnightMoves(const int pos){
    const int offsets[8] = {6,10,15,17,-6,-10,-15,-17};

    for(int move : offsets){
        int newLocation = pos + move;

        //Check if the new location is on the board, before running helper functions

        if(newLocation < 0 || newLocation > 63) continue;

        int columnDifference = abs(convertLocationToColumns(pos) - convertLocationToColumns(newLocation));
        int rowDifference = abs(convertLocationToRows(pos) - convertLocationToRows(newLocation));

        //Check the knight moves in an L shape

        if(!((columnDifference == 1 && rowDifference == 2) || (columnDifference == 2 && rowDifference == 1))){
            continue;
        }
        
        KnightMoves[pos] |= (1ULL << newLocation);
        
    }
}

void computeKingMoves(const int pos){
    const int offsets[8] = {1,7,8,9,-1,-7,-8,-9};

    for(int move : offsets){
        int newLocation = pos + move;

        //Check if the new location is on the board, before running helper functions

        if(newLocation < 0 || newLocation > 63) continue;

        int columnDifference = abs(convertLocationToColumns(pos) - convertLocationToColumns(newLocation));
        int rowDifference = abs(convertLocationToRows(pos) - convertLocationToRows(newLocation));

        //Check the king does not wrap around the board

        if((columnDifference > 1) || (rowDifference > 1)) continue;
            
        KingMoves[pos] |= (1ULL << newLocation);
        
    }
}

void computeQueenMoves(const int pos){

    for(int i = 0; i < 64; i++){
        QueenMoves[i] = (returnSlidingBitBoard(i,true) | returnSlidingBitBoard(i,false));
    }

}

void computeRookMoves(const int pos){

    for(int i = 0; i < 64; i++){
        RookMoves[i] = returnSlidingBitBoard(i,false);
    }

}

void computeBishopMoves(const int pos){

    for(int i = 0; i < 64; i++){
        BishopMoves[i] = returnSlidingBitBoard(i,true);
    }

}

void computePawnMoves(const int pos, bool isWhite){
    const int whiteOffsets[3] = {7,8,9};
    const int blackOffsets[3] = {-7,-8,-9};

    const int *offsets = isWhite ? whiteOffsets : blackOffsets;

    for(int i = 0; i < 3; i++){

        int move = offsets[i];
        int newLocation = pos + move;

        //Check if the new location is on the board, before running helper functions

        if(newLocation < 0 || newLocation > 63) continue;

        int columnDifference = abs(convertLocationToColumns(pos) - convertLocationToColumns(newLocation));
        int rowDifference = abs(convertLocationToRows(pos) - convertLocationToRows(newLocation));

        //Check the pawn does not wrap around the board

        if((columnDifference > 1) || (rowDifference > 1)) continue;
            
        if(isWhite) whitePawnMoves[pos] |= (1ULL << newLocation);
        else blackPawnMoves[pos] |= (1ULL << newLocation);
        
    }

    //If a pawn is on the starting rank they can move forwards 2 spaces, adding the extra space

    if(isWhite && convertLocationToRows(pos) == 1) whitePawnMoves[pos] |= (1ULL << (pos+16));
    else if(!isWhite && convertLocationToRows(pos) == 6) blackPawnMoves[pos] |= (1ULL << (pos-16));

    //TODO: Add enpassant
    
}


uint64_t returnSlidingBitBoard(const int pos, bool diagonalSliding){

    uint64_t slidingBitBoard = 0;
    const int straightOffsets[4] = {1,8,-1,-8};
    const int diagonalOffsets[4] = {7,9,-7,-9};

    const int *offsets = diagonalSliding ? diagonalOffsets : straightOffsets;

   
    for(int i = 0; i < 4; i++){

        int move = offsets[i];

        int newLocation = pos + move;

        //Check if the new location is on the board, before running helper functions

        if(newLocation < 0 || newLocation > 63) continue;


        int column = convertLocationToColumns(pos),
            row = convertLocationToRows(pos),
            newColumn = convertLocationToColumns(newLocation),
            newRow = convertLocationToRows(newLocation);

        //Inital check to ensure we don't instantly wrap around the board

        if(diagonalSliding && (abs(newColumn-column) != abs(newRow - row))) continue;
        else if (!diagonalSliding && (abs(newColumn-column) != 0 && abs(newRow-row) != 0)) continue;


        //Continue sliding until the edge it hit

        while(true){
                
            slidingBitBoard |= (1ULL << newLocation);

            if(diagonalSliding && ((newColumn == 0) || (newRow == 0) || (newColumn == 7) || (newRow == 7))) break;
            else if(!diagonalSliding){
                if((abs(move) == 1) && ((newColumn == 0) || (newColumn == 7))) break;
                if((abs(move) == 8) && ((newRow == 0) || newRow == 7)) break;
            }

            newLocation += move;
            newColumn = convertLocationToColumns(newLocation);
            newRow = convertLocationToRows(newLocation);

        }
        
    }

    return slidingBitBoard;
}
