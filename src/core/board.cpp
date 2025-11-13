#include <bitset>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "board.h"
#include "utils/Types.h"
#include "debug.h"

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];
extern std::unordered_map<std::string, uint64_t[64]> Rays;


Board::Board(const std::string fen, bool isAdversaryWhite){
    isAdversaryWhite = isAdversaryWhite;
    parseFenString(fen);
}


uint64_t Board::getPseudoLegalMoves(const int pos){

    uint64_t *PieceMoves_p = nullptr;
    uint64_t target = 1ULL << pos;

    //If the square is empty
    
    if(!(target & state.whitePieceBitBoard & state.blackPieceBitBoard)) return 0ULL;

    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    int start = 0, end = 0;

    switch(pieceType){
        case Pawn:

            PieceMoves_p = isPieceWhite(pos) ? &whitePawnMoves[pos] : &blackPawnMoves[pos];
            
            break;
        case Knight:
            PieceMoves_p = &KnightMoves[pos];
            break;
        case Bishop:
            PieceMoves_p = &BishopMoves[pos];
            start = 4;
            end = 8;
            break;
        case Rook:
            PieceMoves_p = &RookMoves[pos];
            start = 0;
            end = 4;
            break;
        case King:
            PieceMoves_p = &KingMoves[pos];
            break;
        case Queen:
            PieceMoves_p = &QueenMoves[pos];
            start = 0;
            end = 8;
            break;
        default:
            throw std::runtime_error("Invalid piecetype: ");
            break;
        
    }

    //Find which piece it at the position

    uint64_t blockingRays = 0ULL;
    uint64_t blockers = *PieceMoves_p & (state.whitePieceBitBoard | state.blackPieceBitBoard);

    const std::string directions[8] = {"North","East","South","West","NorthEast","SouthEast","SouthWest","NorthWest"};


    //calculate blockingRays in every direction for the different sliding pieces

    for(int i = start; i < end; i++){

        if(!blockers) break;

        uint64_t blockers1Direction = blockers & Rays[directions[i]][pos];
        int firstBlockerPos;

        if(i==0 || i ==1 || i==4 || i==7){

            //North, East, NorthEast, NorthWest
            firstBlockerPos = __builtin_ctzll(blockers1Direction);
        }else{

            //South, West, SouthEast, SouthWest
            firstBlockerPos = 63 - __builtin_clzll(blockers1Direction);
        }
        
        blockingRays |= Rays[directions[i]][firstBlockerPos];    
    
    }


    //TODO pawn attacks, en passant, castling

    uint64_t pseduoLegalMoves = (*PieceMoves_p) & (~friendlyPieces(pos)) & (~blockingRays);

    visualiseBitBoard(pseduoLegalMoves);

    return pseduoLegalMoves;

}

uint64_t Board::friendlyPieces(int pos){
    return isPieceWhite(pos) ? state.whitePieceBitBoard : state.blackPieceBitBoard;
}


int Board::getPieceEnum(int pos){

    uint64_t target = (1ULL << pos);

    if((state.whitePawnBitBoard & target) || (state.blackPawnBitBoard & target)){
        return 0;
    }
    else if ((state.whiteRookBitBoard & target)||(state.blackRookBitBoard & target)){
        return 1;
    }
    else if ((state.whiteBishopBitBoard & target)||(state.blackBishopBitBoard & target)){
        return 2;
    }
    else if ((state.whiteKnightBitBoard & target)||(state.blackKnightBitBoard & target)){
        return 3;
    }
    else if ((state.whiteQueenBitBoard & target)||(state.blackQueenBitBoard & target)){
        return 4;
    }
    else if ((state.whiteKingBitBoard & target)||(state.blackKingBitBoard & target)){
        return 5;
    }else{
        return -1;
    }

}


bool Board::isAdversaryTurn(){

    return state.whiteToMove ? isAdversaryWhite : !isAdversaryWhite;
    
}

bool Board::isPieceWhite(int pos){
    return (1ULL << pos) & state.whitePieceBitBoard;
}


void Board::updatePieceBitBoards(){

    state.whitePieceBitBoard = state.whitePawnBitBoard | state.whiteBishopBitBoard | state.whiteKnightBitBoard | state.whiteQueenBitBoard | state.whiteKingBitBoard | state.whiteRookBitBoard;
    state.blackPieceBitBoard = state.blackPawnBitBoard | state.blackBishopBitBoard | state.blackKnightBitBoard | state.blackQueenBitBoard | state.blackKingBitBoard | state.blackRookBitBoard;

}

void Board::makeMove(int from, int to, int promotionPiece){

}

void Board::parseFenString(std::string fen){

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

    updatePieceBitBoards();

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

