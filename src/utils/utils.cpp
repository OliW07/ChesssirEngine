#include <iostream>
#include <stdexcept>
#include <vector>


#include "utils/Types.h"



int convertNotationToInt(const std::string &notation){
    
    int file = static_cast<int>(notation[0] - 'a');
    int rank = static_cast<int>(notation[1] - '0');

    if((file > 7) || (file < 0) || (rank > 8) || (rank < 1)){
        throw std::runtime_error("Notation "+notation+"is out of range ");
    }

    return (file + (rank-1)*8);

}

int convertLocationToRows(const int location){
    if(location < 0 || location > 63){
std::cout << "DEBUG: convertLocationToRows called with " << location << std::endl;
        throw std::runtime_error("Location is out of range");
    }
    return location/8;
}

int countOnes(uint64_t state){
    return __builtin_popcountll(state);
}


std::vector<int> convertAlgebraicNotationToMoves(const std::string &notation){

        if(notation.length() < 4 || notation.length() > 5) return {};
            

        std::string from = notation.substr(0,2);
        std::string to = notation.substr(2,2);

        Pieces promotionPiece = None;

        try{

            if(notation.length() == 5){
                promotionPiece = promotionCharToPiece.at(notation[4]);
            } 
            

            std::vector<int> moves = {
                convertNotationToInt(from),
                convertNotationToInt(to),
                static_cast<int>(promotionPiece)
                
            };

            return moves;

        }catch(...){

            return {};
        }

        
}

std::vector<int> getLocationsFromBitBoard(uint64_t bitBoard){
    std::vector<int> locations = {};
    while(bitBoard){
        
        int location = __builtin_ctzll(bitBoard);
        locations.push_back(location);

        //Toggle the bit off
        bitBoard ^= (1ULL << location);
    }

    return locations;
}

RaysDirection convertPositionsToDirections(int pos1, int pos2){

    int pos1Rows = convertLocationToRows(pos1),
        pos2Rows = convertLocationToRows(pos2),
        pos1Columns = convertLocationToColumns(pos1),
        pos2Columns = convertLocationToColumns(pos2),

        rowsDifference = pos1Rows - pos2Rows,
        columnsDifference = pos1Columns - pos2Columns;

    bool isDiagonal = abs(rowsDifference) == abs(columnsDifference);

    //Direction relative to pos1
    RaysDirection direction;
    
    if(rowsDifference == 0 && columnsDifference > 0){
        direction = West;
    }else if(rowsDifference == 0 && columnsDifference < 0){
        direction = East;
    }else if(columnsDifference == 0 && rowsDifference > 0){
        direction = South;
    }else if(columnsDifference == 0 && rowsDifference < 0){
        direction = North;
    }else if(rowsDifference > 0 && columnsDifference > 0 && isDiagonal){
        direction = SouthWest;
    }else if(rowsDifference < 0 && columnsDifference < 0 && isDiagonal){
        direction = NorthEast;
    }else if(rowsDifference > 0 && columnsDifference < 0 && isDiagonal){
        direction = SouthEast;
    }else if(rowsDifference < 0 && columnsDifference > 0 && isDiagonal){
        direction = NorthWest;
    }

    return direction;
}

std::string convertMoveToAlgebraicNotation(int pos){
    char number = convertLocationToRows(pos) + '1';
    char letter = convertLocationToColumns(pos) + 'a';

    std::string result = std::string() + letter + number;
    return result; 
}

int convertLocationToColumns(const int location){
    if(location < 0 || location > 63){
        throw std::runtime_error("Location "+std::to_string(location)+" is out of range");
    }
    return location%8;
}

bool onlyOnePiece(uint64_t state){
    return (__builtin_ctzll(state) + __builtin_clzll(state) == 63);
}

bool posInBounds(int pos){
    return (pos >= 0 && pos <= 63);
}

bool pieceWrapsTheBoard(int pos1, int pos2){
    int columnsDifference = abs(convertLocationToColumns(pos1) - convertLocationToColumns(pos2));
    int rowsDifference = abs(convertLocationToRows(pos1) - convertLocationToRows(pos2));

    return !((columnsDifference < 2) && (rowsDifference < 2));
}



uint8_t convertPieceToBinary(Pieces pieceEnum, bool isWhite){

    return isWhite ? pieceEnum : pieceEnum + 8;

}
