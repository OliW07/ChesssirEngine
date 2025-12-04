
#include <sstream>

#include "fenHelper.h"
#include "utils/Types.h"
#include "board.h"


void parseFenString(std::string fen, BoardState &state){

    uint64_t boardIndex = 56;

    for(int i = 0; i < fen.length(); i++){

        char character = fen[i];
        char uCharacter = static_cast<unsigned char>(character);


        if(std::isdigit(uCharacter)){
            boardIndex += (uCharacter - '0');
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

            //Move position down a line and to the start (left) side
            boardIndex -= 16;
            
            continue;
        }else if(uCharacter == ' '){
            //End of piece data in fen
            fen = fen.substr(i+1);
            break;
        }

        boardIndex += 1;
            
    }

    updatePieceBitBoards(state);
        

    //Parse game data from the end of FEN string

    std::stringstream tokens(fen);
    std::string token;

    tokens >> token;
    state.whiteToMove = (token == "w");
    state.whiteStarts = (token == "w");

    tokens >> token;
    
    for(char c : token){
        
        switch(c){
            case('K'):
                state.castlingRights += 8;
                break;  
                
            case('Q'):
                state.castlingRights += 4;
                break;
            
            case('k'):
                state.castlingRights += 2;
                break;
            
            case('q'):
                state.castlingRights += 1;
                break;
            
        }
	} 


    tokens >> token;

    if(token == "-") state.enPassantSquare = -1;
    else state.enPassantSquare = convertNotationToInt(token);

    tokens >> state.halfMoveClock;
    tokens >> state.fullMoveClock;
}
