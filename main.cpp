#include <iostream>
#include <cctype>
#include <cmath>
#include <bitset>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>

#include "precompute.h"


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
    int enPassantSquare = -1;
    int halfMoveClock = 0;
    int fullMoveClock = 0;
    std::string castlingRights = "";
};

void init(BoardState &state);
void parseFenString(const std::string fen, BoardState &state);
void visualiseBitBoard(const uint64_t &bitBoard);
void visualiseGraphicBoard(BoardState state);
void updatePieceBitBoards(BoardState &state);

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];


const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";



int main(){

    BoardState playingBoard;

    init(playingBoard);

    return 0;
}

void init(BoardState &state){

    std::cout << "Initialising engine... \n";
    
    parseFenString(STARTING_FEN,state);
    precomputeBitBoardMoves();
}

void parseFenString(std::string fen, BoardState &state){

    uint64_t boardIndex = 63;

    for(int i = 0; i < fen.length(); i++){

        char character = fen[i];
        char uCharacter = static_cast<unsigned char>(character);


        if(std::isdigit(uCharacter)){
            boardIndex -= (uCharacter - '0');
            continue;

        }else if(std::isalpha(uCharacter)){

            uint64_t *pieceBitBoard = nullptr;

            bool isWhite = std::isupper(uCharacter);

            switch (std::tolower(uCharacter)) {
                case 'r':
                    pieceBitBoard = isWhite ? &state.whiteRookBitBoard : &state.blackRookBitBoard;
                    break;
                case 'n':
                    pieceBitBoard = isWhite ? &state.whiteKnightBitBoard : &state.blackKnightBitBoard;
                    break;
                case 'b':
                    pieceBitBoard = isWhite ? &state.whiteBishopBitBoard : &state.blackBishopBitBoard;
                    break;
                case 'q':
                    pieceBitBoard = isWhite ? &state.whiteQueenBitBoard : &state.blackQueenBitBoard;
                    break;
                case 'k':
                    pieceBitBoard = isWhite ? &state.whiteKingBitBoard : &state.blackKingBitBoard;
                    break;
                case 'p':
                    pieceBitBoard = isWhite ? &state.whitePawnBitBoard : &state.blackPawnBitBoard;
                    break;
                default:
                    throw std::runtime_error("Unknown piece character in FEN.");
                        
            }

            *pieceBitBoard |= (1ULL << boardIndex);

        }else if(uCharacter == '/'){
            //Don't increment the counter as / is not representitive of a piece.
            continue;
        }else if(uCharacter == ' '){
            //End of piece data in fen
            fen = fen.substr(i+1);
            break;
        }

        boardIndex -= 1;
            
    }

    updatePieceBitBoards(state);

    //Parse game data from the end of FEN string

    std::stringstream tokens(fen);
    std::string token;

    tokens >> token;
    state.whiteToMove = (token == "w");

    tokens >> state.castlingRights;
    
    tokens >> token;

    if(token == "-") state.enPassantSquare = -1;
    else state.enPassantSquare = convertNotationToInt(token);

    tokens >> state.halfMoveClock;
    tokens >> state.fullMoveClock;



}



/*--------------------------------------------
              Debugger functions
--------------------------------------------*/

void visualiseBitBoard(const uint64_t &bitBoard){
    std::bitset<64> boardBitSet(bitBoard);
    std::string bitString = boardBitSet.to_string();

    std::cout << "\n";

    for(int i = 0; i < 8; i++){
        std::string ss = bitString.substr(8*i, 8);

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

    for(int i = 0; i < 64; i++){

        //At each location, check all possible piece and if present add to array

        bool whitePawn = state.whitePawnBitBoard & 1,
            whiteBishop = state.whiteBishopBitBoard & 1,
            whiteKnight = state.whiteKnightBitBoard & 1,
            whiteRook = state.whiteRookBitBoard & 1,
            whiteKing = state.whiteKingBitBoard & 1,
            whiteQueen = state.whiteQueenBitBoard & 1,
            blackPawn = state.blackPawnBitBoard & 1,
            blackBishop = state.blackBishopBitBoard & 1,
            blackKnight = state.blackKnightBitBoard & 1,
            blackRook = state.blackRookBitBoard & 1,
            blackKing = state.blackKingBitBoard & 1,
            blackQueen = state.blackQueenBitBoard & 1;

        state.whitePawnBitBoard >>= 1;
        state.whiteBishopBitBoard >>= 1;
        state.whiteKnightBitBoard >>= 1;
        state.whiteRookBitBoard >>= 1;
        state.whiteKingBitBoard >>= 1;
        state.whiteQueenBitBoard >>= 1;
        state.blackPawnBitBoard >>= 1;
        state.blackKnightBitBoard >>= 1;
        state.blackBishopBitBoard >>= 1;
        state.blackRookBitBoard >>= 1;
        state.blackKingBitBoard >>= 1;
        state.blackQueenBitBoard >>= 1;

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

        if((i+1)%8==0) std::cout << "\n\n";

    }

    std::cout << "\n";
}



/*--------------------------------------------
              Helper functions
--------------------------------------------*/

int convertNotationToInt(const std::string &notation){
    
    int file = static_cast<int>(notation[0] - 'a');
    int rank = static_cast<int>(notation[1] - '0');

    if((file > 7) || (file < 0) || (rank > 8) || (rank < 1)){
        throw std::runtime_error("Notation "+notation+"is out of range ");
    }

    return (file + (rank-1)*8);

}

int convertLocationToRows(const int location){
    if(location < 0 || location > 64){
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

void updatePieceBitBoards(BoardState &state){

    state.whitePieceBitBoard = state.whitePawnBitBoard | state.whiteBishopBitBoard | state.whiteKnightBitBoard | state.whiteQueenBitBoard | state.whiteKingBitBoard | state.whiteRookBitBoard;
    state.blackPieceBitBoard = state.blackPawnBitBoard | state.blackBishopBitBoard | state.blackKnightBitBoard | state.blackQueenBitBoard | state.blackKingBitBoard | state.blackRookBitBoard;

}





