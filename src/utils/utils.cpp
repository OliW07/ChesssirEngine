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
        throw std::runtime_error("Location is out of range");
    }
    return location/8;
}

std::vector<int> convertAlgebraicNotationToMoves(const std::string &notation){

        if(notation.length() < 4 || notation.length() > 5) return {};
            

        std::string from = notation.substr(0,2);
        std::string to = notation.substr(2,2);

        int promotionType = -1;

        try{

            if(notation.length() == 5){
                promotionType = promotionPieceCharIntConversion.at(notation[4]);
            } 
            

            std::vector<int> moves = {
                convertNotationToInt(from),
                convertNotationToInt(to),
                promotionType
                
            };

            return moves;

        }catch(...){

            return {};
        }

        
}


int convertLocationToColumns(const int location){
    if(location < 0 || location > 63){
        throw std::runtime_error("Location "+std::to_string(location)+" is out of range");
    }
    return location%8;
}



