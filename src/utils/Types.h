#ifndef TYPES
#define TYPES

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


struct BoardState{
    uint64_t
        whitePawnBitBoard = 0ULL,
        whiteKnightBitBoard = 0ULL,
        whiteBishopBitBoard = 0ULL,
        whiteRookBitBoard = 0ULL,
        whiteKingBitBoard = 0ULL,
        whiteQueenBitBoard = 0ULL,
        whitePieceBitBoard = 0ULL,
        blackPawnBitBoard = 0ULL,
        blackKnightBitBoard = 0ULL,
        blackBishopBitBoard = 0ULL,
        blackRookBitBoard = 0ULL,
        blackKingBitBoard = 0ULL,
        blackQueenBitBoard = 0ULL,
        blackPieceBitBoard = 0ULL;

    bool whiteToMove = 0;
    bool whiteStarts = 1;
    int enPassantSquare = -1;
    int halfMoveClock = 0;
    int fullMoveClock = 0;
    std::string castlingRights = "";
};

const std::unordered_map<char,int> promotionPieceCharIntConversion = {
    {'q',4},
    {'r',1},
    {'n',3},
    {'b',2}
};


int convertNotationToInt(const std::string &notation);
int convertLocationToRows(const int location);
int convertLocationToColumns(const int location);
int countOnes(uint64_t state);

bool onlyOnePiece(uint64_t state);
bool posInBounds(int pos);
bool pieceWrapsTheBoard(int pos1, int pos2);

std::string convertPositionsToDirections(int pos1, int pos2);

std::vector<int> convertAlgebraicNotationToMoves(const std::string &notation);
std::vector<int> getLocationsFromBitBoard(uint64_t bitBoard);


#endif