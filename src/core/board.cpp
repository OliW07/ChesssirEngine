#include <cstdlib>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <algorithm>

#include "fenHelper.h"
#include "board.h"
#include "Types.h"
#include "precompute.h"
#include "utils/Types.h"
#include "debug.h"

using namespace precomputedData;

Board::Board(const std::string fen, bool isAdversaryWhite){
    this->isAdversaryWhite = isAdversaryWhite;
    parseFenString(fen,state);
}


uint64_t Board::getFriendlyPieces(int pos){
    return isPieceWhite(pos) ? state.whitePieceBitBoard : state.blackPieceBitBoard;
}

uint64_t Board::getKingLocation(bool isWhite){
    return isWhite ? __builtin_ctzll(state.whiteKingBitBoard) : __builtin_ctzll(state.blackKingBitBoard);
}

uint64_t Board::getEnemyPieces(int pos){
    return isPieceWhite(pos) ? state.blackPieceBitBoard : state.whitePieceBitBoard;
}

uint64_t Board::getRay(int pos1, int pos2){
    
    RaysDirection direction = convertPositionsToDirections(pos1,pos2);

    uint64_t ray = rays[direction][pos1];
   
    ray &= ~rays[direction][pos2];

    return ray;
}

uint64_t* Board::getBitBoardFromPiece(int pieceEnum, bool isWhite){
    uint64_t *pieceBitBoard = nullptr;
    switch(pieceEnum){
        case 0:
            pieceBitBoard = isWhite ? &state.whitePawnBitBoard : &state.blackPawnBitBoard;
            break;
        case 1:
            pieceBitBoard = isWhite ? &state.whiteRookBitBoard : &state.blackRookBitBoard;
            break;
        case 2:
            pieceBitBoard = isWhite ? &state.whiteBishopBitBoard : &state.blackBishopBitBoard;
            break;
        case 3:
            pieceBitBoard = isWhite ? &state.whiteKnightBitBoard : &state.blackKnightBitBoard;
            break;
        case 4:
            pieceBitBoard = isWhite ? &state.whiteQueenBitBoard : &state.blackQueenBitBoard;
            break;
        case 5:
            pieceBitBoard = isWhite ? &state.whiteKingBitBoard : &state.blackKingBitBoard;
            break;
        default:
            throw std::runtime_error("Invalid piece enum");
            break;
    }
    return pieceBitBoard;
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

int Board::getFirstBlocker(int pos, RaysDirection direction){
    
    uint64_t blockers = rays[direction][pos] & (state.whitePieceBitBoard | state.blackPieceBitBoard);

    if(!blockers) return -1; 

    if(direction == North || direction == East || direction == NorthEast || direction == NorthWest) return __builtin_ctzll(blockers);
         
    return 63 - __builtin_clzll(blockers);
    
}



bool Board::isAdversaryTurn(){

    return state.whiteToMove ? isAdversaryWhite : !isAdversaryWhite;
    
}

bool Board::isPieceWhite(int pos){
    return (1ULL << pos) & state.whitePieceBitBoard;
}

bool Board::isSquareEmpty(int pos){

    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.whitePieceBitBoard | state.blackPieceBitBoard;

    
    return !(target & allPieces);

}



void Board::makeMove(int from, int to, int promotionPieceType){

    Pieces pieceType = (Pieces)getPieceEnum(from);
    bool isWhite = isPieceWhite(from);

    uint64_t *pieceBitBoard = getBitBoardFromPiece((int)pieceType,isWhite);


    if((1ULL << to) & (state.whitePieceBitBoard | state.blackPieceBitBoard)){
        //Capturing a piece
        
        Pieces capturedPiece = (Pieces)getPieceEnum(to);

        if(capturedPiece == Rook && state.castlingRights > 0){
            switch(to){
                case(0):
                    state.castlingRights &= ~4;
                    break;
                case(7):
                    state.castlingRights &= ~8;
                    break;
                case(56):
                    state.castlingRights &= ~1;
                    break;
                case(63):
                    state.castlingRights &= ~2;
            }
        }

        uint64_t *capturedBitBoard = getBitBoardFromPiece(capturedPiece,!isWhite);

        //Toggle off the captured piece
        *capturedBitBoard ^= (1ULL << to);

        state.halfMoveClock = -1;
    }

    else if(pieceType == Pawn && to == state.enPassantSquare && state.enPassantSquare != -1){
        uint64_t *capturedBitBoard = getBitBoardFromPiece(Pawn,!isWhite);
        int capturePos = isWhite ? to - 8 : to + 8;
        *capturedBitBoard ^= (1ULL << capturePos);
        state.halfMoveClock = -1;
    }
    *pieceBitBoard |= (1ULL << to);
    *pieceBitBoard ^= (1ULL << from);

    if((pieceType == Pawn)){

        if(abs(convertLocationToRows(from) - convertLocationToRows(to)) == 2){
            //Set the enpassant square
            state.enPassantSquare = isWhite ? from+8 : from-8;
            
        }else{
            state.enPassantSquare = -1;
        }

        if((isWhite && convertLocationToRows(to) == 7) || (!isWhite && convertLocationToRows(to) == 0)){
            //Pawn promotes
            uint64_t *newPieceBitBoard = getBitBoardFromPiece(promotionPieceType,isWhite);
            *newPieceBitBoard |= (1ULL << to);
            *pieceBitBoard ^= (1ULL << to);
        }   
        state.halfMoveClock = -1;
        
        
    }else{
        state.enPassantSquare = -1;
    }

    if(pieceType == King && state.castlingRights > 0){

        if(isWhite){
            state.castlingRights &= ~12;
        }else{
            state.castlingRights &= ~3;
        }


    }
    if(pieceType == Rook && state.castlingRights > 0){
        //Remove coresponding castling letter
        if(isWhite && from == 0) state.castlingRights &= ~4;
        if(isWhite && from == 7) state.castlingRights &= ~8;
        if(!isWhite && from == 63) state.castlingRights &= ~2;
        if(!isWhite && from == 56) state.castlingRights &= ~1;
    }

    //If castling, move the rook
    
    if(pieceType == King && (abs(convertLocationToColumns(to) - convertLocationToColumns(from)) > 1)){
        
        switch(to){
            case(2):
                state.whiteRookBitBoard &= ~(1ULL << 0);
                state.whiteRookBitBoard |= (1ULL << 3);
                break;
            case(6):
                state.whiteRookBitBoard &= ~(1ULL << 7);
                state.whiteRookBitBoard |= (1ULL << 5);
                break;
            case(58):
                state.blackRookBitBoard &= ~(1ULL << 56);
                state.blackRookBitBoard |= (1ULL << 59);
                break;
            case(62):
                state.blackRookBitBoard &= ~(1ULL << 63);
                state.blackRookBitBoard |= (1ULL << 61);
                break;
            default:
                std::runtime_error("Error, the king is making an illegal move");
        }

    }


    updatePieceBitBoards(state);


    if(state.whiteToMove != state.whiteStarts) state.fullMoveClock++;
    state.halfMoveClock++;

    state.whiteToMove = !state.whiteToMove;
    
}

