#include <cctype>
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
            Pieces pieceType = notationToPieceMap.at(std::tolower(uCharacter));

            pieceBitBoard = &state.bitboards[isWhite][pieceType];
            uint64_t pieceMask = (1ULL << boardIndex);

            *pieceBitBoard |= pieceMask;
            state.occupancy[isWhite] ^= pieceMask;
            state.occupancy[Both] ^= pieceMask;
            state.mailBox[boardIndex] = convertPieceToBinary(pieceType,isWhite);
            state.pieceList.addPiece(boardIndex, isWhite ? White : Black);

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
