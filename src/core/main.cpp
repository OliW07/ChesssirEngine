#include <iostream>
#include <cctype>
#include <cmath>
#include <bitset>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>


#include "data/precompute.h"
#include "utils/Types.h"





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


void updatePieceBitBoards(BoardState &state){

    state.whitePieceBitBoard = state.whitePawnBitBoard | state.whiteBishopBitBoard | state.whiteKnightBitBoard | state.whiteQueenBitBoard | state.whiteKingBitBoard | state.whiteRookBitBoard;
    state.blackPieceBitBoard = state.blackPawnBitBoard | state.blackBishopBitBoard | state.blackKnightBitBoard | state.blackQueenBitBoard | state.blackKingBitBoard | state.blackRookBitBoard;

}

