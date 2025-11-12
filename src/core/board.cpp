#include <bitset>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>
#include <vector>

#include "board.h"
#include "utils/Types.h"

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];


Board::Board(const std::string fen, bool isAdversaryWhite){
    isAdversaryWhite = isAdversaryWhite;
    parseFenString(fen);
}

void Board::makeMove(int from, int to, int promotionPiece){

}

bool Board::isWhite(int pos){
    return (1ULL << pos) & state.whitePieceBitBoard;
}

bool Board::friendlyPieces(int pos){
    return isWhite(pos) ? state.whitePieceBitBoard : state.blackPieceBitBoard;
}

uint64_t Board::getPseduoLegalMoves(int pos){

    uint64_t *PieceMoves = nullptr;
    uint64_t target = 1ULL << pos;


    //Find which piece it at the position

    if(state.whitePawnBitBoard & target){
        PieceMoves = whitePawnMoves;
    }
    else if (state.blackPawnBitBoard & target){
        PieceMoves = blackPawnMoves;
    }
    else if ((state.whiteKnightBitBoard & target)||(state.blackKnightBitBoard & target)){
        PieceMoves = KnightMoves;
    }
    else if ((state.whiteBishopBitBoard & target)||(state.blackBishopBitBoard & target)){
        PieceMoves = BishopMoves;
    }
    else if ((state.whiteRookBitBoard & target)||(state.blackRookBitBoard & target)){
        PieceMoves = RookMoves;
    }
    else if ((state.whiteQueenBitBoard & target)||(state.blackQueenBitBoard & target)){
        PieceMoves = QueenMoves;
    }
    else if ((state.whiteKingBitBoard & target)||(state.blackKingBitBoard & target)){
        PieceMoves = KingMoves;
    }
    else{

        //No piece at the position
        return 0ULL;
    }

    //TODO filter sliding blockers

    uint64_t pseduoLegalMoves = *PieceMoves & (~friendlyPieces(pos));

    return pseduoLegalMoves;

}

void Board::parseFenString(std::string fen){

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

bool Board::isAdversaryTurn(){

    return state.whiteToMove ? isAdversaryWhite : !isAdversaryWhite;
    
}

void updatePieceBitBoards(BoardState &state){

    state.whitePieceBitBoard = state.whitePawnBitBoard | state.whiteBishopBitBoard | state.whiteKnightBitBoard | state.whiteQueenBitBoard | state.whiteKingBitBoard | state.whiteRookBitBoard;
    state.blackPieceBitBoard = state.blackPawnBitBoard | state.blackBishopBitBoard | state.blackKnightBitBoard | state.blackQueenBitBoard | state.blackKingBitBoard | state.blackRookBitBoard;

}
