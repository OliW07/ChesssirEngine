#include <stdexcept>


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

int convertLocationToColumns(const int location){
    if(location < 0 || location > 63){
        throw std::runtime_error("Location "+std::to_string(location)+" is out of range");
    }
    return location%8;
}
