
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <map>
#include "precompute.h"
#include "utils/Types.h"

uint64_t KnightMoves[64] = {};
uint64_t KingMoves[64] = {};
uint64_t QueenMoves[64] = {};
uint64_t RookMoves[64] = {};
uint64_t BishopMoves[64] = {};
uint64_t whitePawnMoves[64] = {};
uint64_t blackPawnMoves[64] = {};

std::unordered_map<std::string, uint64_t[64]> Rays;


void computeKnightMoves(const int pos);
void computeKingMoves(const int pos);
void computeQueenMoves(const int pos);
void computeRookMoves(const int pos);
void computeBishopMoves(const int pos);
void computePawnMoves(const int pos, bool isWhite);
void computeSlidingRays(const int pos);




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
        computeSlidingRays(i);
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

    QueenMoves[pos] = (Rays["North"][pos] | Rays["East"][pos] | Rays["South"][pos] | Rays["West"][pos] | Rays["NorthEast"][pos] | Rays["SouthEast"][pos] | Rays["SouthWest"][pos] | Rays["NorthWest"][pos]);
    
}

void computeRookMoves(const int pos){

    RookMoves[pos] = (Rays["North"][pos] | Rays["East"][pos] | Rays["South"][pos] | Rays["West"][pos]);
}

void computeBishopMoves(const int pos){

    BishopMoves[pos] = (Rays["NorthEast"][pos] | Rays["SouthEast"][pos] | Rays["SouthWest"][pos] | Rays["NorthWest"][pos]);
    
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
    
}

void computeSlidingRays(const int pos){
    
    const std::map<std::string,int> offsets = {
        {"North",8},
        {"NorthEast",9},
        {"East",1},
        {"SouthEast",-7},
        {"South",-8},
        {"SouthWest",-9},
        {"West",-1},
        {"NorthWest",7}

    };

    for (auto const &[direction, offset] : offsets){

        int newLocation = pos + offset;

        bool diagonalSliding = (direction == "NorthEast" || direction == "SouthEast" || direction == "SouthWest" || direction == "NorthWest");

        

        uint64_t *RayBitBoard_p = &Rays[direction][pos];

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


