#ifndef TYPES
#define TYPES

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <array>

struct BoardState{

    uint64_t bitboards[2][6] = {0ULL};
    uint64_t occupancy[3] = {0ULL};
    
    bool whiteToMove = 0;
    bool whiteStarts = 1;
    int enPassantSquare = -1;
    int halfMoveClock = 0;
    int fullMoveClock = 0;

    //least three significant bits represent the pieceType enum 0-5 for each piece, the 4th represents colour, white = 0
    std::array<uint8_t,64> mailBox = {}; 

    //least four signficant bits represent, White Kingside, White Queenside, BlackKingside, BlackQueenside
    uint8_t castlingRights = 0;
};

enum Pieces {Bishop,Queen,Rook,King,Pawn,Knight,None};
enum RaysDirection {North,South,East,West,NorthEast,SouthEast,NorthWest,SouthWest};
enum Colours {Black,White,Both};


const std::unordered_map<char,Pieces> promotionCharToPiece = {
    {'q',Queen},
    {'r',Rook},
    {'n',Knight},
    {'b',Bishop}
};


const std::unordered_map<char,Pieces> notationToPieceMap = {
    {'k',King},
    {'q',Queen},
    {'r',Rook},
    {'b',Bishop},
    {'n',Knight},
    {'p',Pawn}
};


const std::map<RaysDirection,int> Compass = {
        {North,8},
        {NorthEast,9},
        {East,1},
        {SouthEast,-7},
        {South,-8},
        {SouthWest,-9},
        {West,-1},
        {NorthWest,7}

};
int convertNotationToInt(const std::string &notation);
int convertLocationToRows(const int location);
int convertLocationToColumns(const int location);

int countOnes(uint64_t state);

bool onlyOnePiece(uint64_t state);
bool posInBounds(int pos);
bool pieceWrapsTheBoard(int pos1, int pos2);

uint8_t convertPieceToBinary(Pieces pieceEnum, bool isWhite);

RaysDirection convertPositionsToDirections(int pos1, int pos2);


std::string convertMoveToAlgebraicNotation(int pos);

std::vector<int> convertAlgebraicNotationToMoves(const std::string &notation);
std::vector<int> getLocationsFromBitBoard(uint64_t bitBoard);


#endif
