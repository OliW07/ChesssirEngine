
#include <iostream>
#include <cmath>
#include "precompute.h"
#include "utils/Types.h"

namespace precomputedData {

    uint64_t knightMoves[64] = {};
    uint64_t kingMoves[64] = {};
    uint64_t queenMoves[64] = {};
    uint64_t rookMoves[64] = {};
    uint64_t bishopMoves[64] = {};
    uint64_t whitePawnMoves[64] = {};
    uint64_t blackPawnMoves[64] = {};
    uint64_t whitePawnAttacks[64] = {};
    uint64_t blackPawnAttacks[64] = {};

    uint64_t rays[8][64] = {};


    uint64_t rankMasks[8] = {
        0x00000000000000FF,  
        0x000000000000FF00,  
        0x0000000000FF0000,  
        0x00000000FF000000,  
        0x000000FF00000000, 
        0x0000FF0000000000, 
        0x00FF000000000000, 
        0xFF00000000000000   
    };
};

void computeKnightMoves(const int pos);
void computeKingMoves(const int pos);
void computeQueenMoves(const int pos);
void computeRookMoves(const int pos);
void computeBishopMoves(const int pos);
void computePawnMoves(const int pos, bool isWhite);
void computeSlidingRays(const int pos);




void precomputeBitBoardMoves(){

    
    for(int i = 0; i < 64; i++){
        
        computeSlidingRays(i);
        computeKnightMoves(i);
        computeKingMoves(i);
        computeQueenMoves(i);
        computeRookMoves(i);
        computeBishopMoves(i);
        computePawnMoves(i,true);
        computePawnMoves(i,false);

        
        
    }
    
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
        
        precomputedData::knightMoves[pos] |= (1ULL << newLocation);
        
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
            
        precomputedData::kingMoves[pos] |= (1ULL << newLocation);
        
    }
}

void computeQueenMoves(const int pos){

    precomputedData::queenMoves[pos] = (precomputedData::rays[North][pos] | precomputedData::rays[East][pos] | precomputedData::rays[South][pos] | precomputedData::rays[West][pos] | precomputedData::rays[NorthEast][pos] | precomputedData::rays[SouthEast][pos] | precomputedData::rays[SouthWest][pos] | precomputedData::rays[NorthWest][pos]);
    
}

void computeRookMoves(const int pos){

    precomputedData::rookMoves[pos] = (precomputedData::rays[North][pos] | precomputedData::rays[East][pos] | precomputedData::rays[South][pos] | precomputedData::rays[West][pos]);
}

void computeBishopMoves(const int pos){

    precomputedData::bishopMoves[pos] = (precomputedData::rays[NorthEast][pos] | precomputedData::rays[SouthEast][pos] | precomputedData::rays[SouthWest][pos] | precomputedData::rays[NorthWest][pos]);
    
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
        
        if(i == 1){
            //Straight movement, pawn move
            if(isWhite) precomputedData::whitePawnMoves[pos] |= (1ULL << newLocation);
            else precomputedData::blackPawnMoves[pos] |= (1ULL << newLocation);
        }else{
            //Diagonal movement, pawn attack
            if(isWhite) precomputedData::whitePawnAttacks[pos] |= (1ULL << newLocation);
            else precomputedData::blackPawnAttacks[pos] |= (1ULL << newLocation);
        }
        
        
    }

    //If a pawn is on the starting rank they can move forwards 2 spaces, adding the extra space

    if(isWhite && convertLocationToRows(pos) == 1) precomputedData::whitePawnMoves[pos] |= (1ULL << (pos+16));
    else if(!isWhite && convertLocationToRows(pos) == 6) precomputedData::blackPawnMoves[pos] |= (1ULL << (pos-16));
    
}

void computeSlidingRays(const int pos){
    

    for (auto const &[direction, offset] : Compass){

        int newLocation = pos + offset;

        bool diagonalSliding = (direction == NorthEast || direction == SouthEast || direction == SouthWest || direction == NorthWest);

        

        uint64_t *RayBitBoard_p = &precomputedData::rays[direction][pos];

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
                
            *RayBitBoard_p |= (1ULL << newLocation);

            if(diagonalSliding && ((newColumn == 0) || (newRow == 0) || (newColumn == 7) || (newRow == 7))) break;
            else if(!diagonalSliding){
                if((abs(offset) == 1) && ((newColumn == 0) || (newColumn == 7))) break;
                if((abs(offset) == 8) && ((newRow == 0) || newRow == 7)) break;
            }

            newLocation += offset;
            newColumn = convertLocationToColumns(newLocation);
            newRow = convertLocationToRows(newLocation);

        }

    }

}


