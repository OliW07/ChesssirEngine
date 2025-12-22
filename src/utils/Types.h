#ifndef TYPES
#define TYPES

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <array>

enum Pieces {Bishop,Queen,Rook,King,Pawn,Knight,None};
enum RaysDirection {North,South,East,West,NorthEast,SouthEast,NorthWest,SouthWest};
enum Colours {Black,White,Both};

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class PieceList {
    public:
        int list[2][16];
        int mapBoardLocToList[64] = {-1};
        int pieceCount[2]; 

        PieceList() {
            std::fill(std::begin(mapBoardLocToList), std::end(mapBoardLocToList), -1);
            
            pieceCount[0] = 0;
            pieceCount[1] = 0;
        }

        void addPiece(int pos, Colours colour);
        void removePiece(int pos, Colours colour);
        void movePiece(int to, int from, Colours colour);
        
};


struct BoardState{

    uint64_t bitboards[3][6] = {0ULL};
    uint64_t occupancy[3] = {0ULL};
    
    bool whiteToMove = 0;
    bool whiteStarts = 1;
    int enPassantSquare = -1;
    int halfMoveClock = 0;
    int fullMoveClock = 0;

    //least three significant bits represent the pieceType enum 0-5 for each piece, the 4th represents colour, white = 0
    std::array<uint8_t,64> mailBox = {}; 

    PieceList pieceList;

    //least four signficant bits represent, White Kingside, White Queenside, BlackKingside, BlackQueenside
    uint8_t castlingRights = 0;
};


struct Move {
    
    bool nullMove = false;

    int to;
    int from;
    Pieces promotionPiece = None;
    
};

struct SearchInfo {
    int wtime = -1;
    int btime = -1;
    int winc = 0;
    int binc = 0;
    int movestogo = -1;
    int depth = -1;
    int movetime = -1;
    bool infinite = false;
};

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

const std::unordered_map<Pieces,char> pieceToNotation = {
    {King,'k'},
    {Queen,'q'},
    {Rook,'r'},
    {Bishop,'b'},
    {Knight,'n'},
    {Pawn,'p'}
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


Colours getSquareColour(int pos);

std::string convertMoveToAlgebraicNotation(Move move);

Move convertAlgebraicNotationToMove(const std::string &notation);
std::vector<int> getLocationsFromBitBoard(uint64_t bitBoard);


#endif
