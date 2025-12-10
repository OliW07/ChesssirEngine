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
    return isPieceWhite(pos) ? state.occupancy[White]: state.occupancy[Black];
}

uint64_t Board::getKingLocation(bool isWhite){
    return isWhite ? __builtin_ctzll(state.bitboards[White][King]) : __builtin_ctzll(state.bitboards[Black][King]);
}

uint64_t Board::getEnemyPieces(int pos){
    return isPieceWhite(pos) ? state.occupancy[Black]: state.occupancy[White];
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
            pieceBitBoard = isWhite ? &state.bitboards[White][Bishop] : &state.bitboards[Black][Bishop];
            break;
        case 1:
            pieceBitBoard = isWhite ? &state.bitboards[White][Queen] : &state.bitboards[Black][Queen];
            break;
        case 2:
            pieceBitBoard = isWhite ? &state.bitboards[White][Rook] : &state.bitboards[Black][Rook];
            break;
        case 3:
            pieceBitBoard = isWhite ? &state.bitboards[White][King] : &state.bitboards[Black][King];
            break;
        case 4:
            pieceBitBoard = isWhite ? &state.bitboards[White][Pawn] : &state.bitboards[Black][Pawn];
            break;
        case 5:
            pieceBitBoard = isWhite ? &state.bitboards[White][Knight] : &state.bitboards[Black][Knight];
            break;
        default:
            throw std::runtime_error("Invalid piece enum");
            break;
    }
    return pieceBitBoard;
}



int Board::getPieceEnum(int pos){
      //Toggle off the colour of piece, to get left with the enum
      return state.mailBox[pos] & ~8; 

}

int Board::getFirstBlocker(int pos, RaysDirection direction){
    
    uint64_t blockers = rays[direction][pos] & (state.occupancy[White] | state.occupancy[Black]);

    if(!blockers) return -1; 

    if(direction == North || direction == East || direction == NorthEast || direction == NorthWest) return __builtin_ctzll(blockers);
         
    return 63 - __builtin_clzll(blockers);
    
}



bool Board::isAdversaryTurn(){

    return state.whiteToMove ? isAdversaryWhite : !isAdversaryWhite;
    
}

bool Board::isPieceWhite(int pos){
    return (1ULL << pos) & state.occupancy[White];
}

bool Board::isSquareEmpty(int pos){

    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.occupancy[White] | state.occupancy[Black];

    
    return !(target & allPieces);

}



void Board::makeMove(Move move){

    Pieces pieceType = (Pieces)getPieceEnum(move.from);
    bool isWhite = isPieceWhite(move.from);

    uint64_t *pieceBitBoard = getBitBoardFromPiece((int)pieceType,isWhite);


    if((1ULL << move.to) & state.occupancy[Both]){

        handleCapture(move.from,move.to,isWhite);
    }


    else if(pieceType == Pawn && move.to == state.enPassantSquare && state.enPassantSquare != -1){
       handleEnpassant(move.from,move.to,isWhite);
    }
   
    uint64_t moveMask = (1ULL << move.to) | (1ULL << move.from);

    *pieceBitBoard ^= moveMask;

    state.mailBox[move.to] = convertPieceToBinary(pieceType, isWhite);
    state.mailBox[move.from] = 0;

    state.pieceList.movePiece(move.to,move.from,(Colours)isWhite);

    if(pieceType == Pawn){

       handlePawnMove(move.from,move.to,isWhite,move.promotionPiece,*pieceBitBoard);
        
    }else{
        state.enPassantSquare = -1;
    }

    if((pieceType == King || pieceType == Rook) && state.castlingRights > 0){

       updateCastlingRights(move.from, isWhite, pieceType);


    }

    //If castling, move the rook
    
    if(pieceType == King && (abs(convertLocationToColumns(move.to) - convertLocationToColumns(move.from)) > 1)){
       handleRookCastle(move.to); 

    }

    state.occupancy[isWhite] ^= (1ULL << move.to);
    state.occupancy[isWhite] ^= (1ULL << move.from);
    state.occupancy[Both] ^= (1ULL << move.from);
    state.occupancy[Both] |= (1ULL << move.to);

    if(state.whiteToMove != state.whiteStarts) state.fullMoveClock++;
    state.halfMoveClock++;

    state.whiteToMove = !state.whiteToMove;
    
}

void Board::handleCapture(int from, int to,bool isWhite){

    //Capturing a piece
    
    Pieces capturedPiece = (Pieces)getPieceEnum(to);
if(capturedPiece == King) std::cout << "DEBUG: King captured at " << to << " by move from " << from << std::endl;

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
    state.occupancy[!isWhite] ^= (1ULL << to);
    state.mailBox[to] = convertPieceToBinary(capturedPiece,isWhite);
    state.pieceList.removePiece(to,(Colours)!isWhite);

    state.halfMoveClock = -1;
}

void Board::handleEnpassant(int from, int to, bool isWhite){

    uint64_t *capturedBitBoard = getBitBoardFromPiece(Pawn,!isWhite);
    int capturePos = isWhite ? to - 8 : to + 8;
    uint64_t captureMask = 1ULL << capturePos;


    *capturedBitBoard ^= captureMask;
    state.occupancy[!isWhite] ^= captureMask;
    state.occupancy[Both] ^= captureMask;
    state.mailBox[capturePos] = 0;
    state.pieceList.removePiece(capturePos,(Colours)!isWhite);
    state.halfMoveClock = -1;
}

void Board::updateCastlingRights(int from, bool isWhite, Pieces pieceType){

    if(isWhite){

        if((pieceType == Rook && from == 0) || pieceType == King){
            state.castlingRights &= ~4;
        }

        if((pieceType == Rook && from == 7) || pieceType == King){
            state.castlingRights &= ~8;
        }
    }

    else{
        
        if((pieceType == Rook && from == 63) || pieceType == King){
            state.castlingRights &= ~2;
        }

        if((pieceType == Rook && from == 56) || pieceType == King){
            state.castlingRights &= ~1;
        }
    }

}

void Board::handlePawnMove(int from, int to, bool isWhite, Pieces promotionPiece, uint64_t &pawnBitBoard){

        if(abs(convertLocationToRows(from) - convertLocationToRows(to)) == 2){
            //Set the enpassant square
            state.enPassantSquare = isWhite ? from+8 : from-8;
            
        }else{
            state.enPassantSquare = -1;
        }

        if((isWhite && convertLocationToRows(to) == 7) || (!isWhite && convertLocationToRows(to) == 0)){
            //Pawn promotes
            uint64_t *newPieceBitBoard = getBitBoardFromPiece(promotionPiece,isWhite);
            *newPieceBitBoard |= (1ULL << to);
            pawnBitBoard ^= (1ULL << to);
            state.mailBox[to] = convertPieceToBinary(promotionPiece,isWhite);
            //Dont need to change piecelist as the location of the promotion piece is the same as where the pawn just moved to.
        }   
        state.halfMoveClock = -1;
}


void Board::handleRookCastle(int newKingLoc){
    
    switch(newKingLoc){
        case(2):
            {uint64_t rookMoveMask = (1ULL << 0) | (1ULL << 3);
            state.bitboards[White][Rook] ^= rookMoveMask;

            state.occupancy[White] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            
            state.mailBox[3] = int(Rook);
            state.mailBox[0] = 0;}

            state.pieceList.movePiece(3,0,White);

            break;
        case(6):
            {uint64_t rookMoveMask = (1ULL << 5) | (1ULL << 7);
            state.bitboards[White][Rook] ^= rookMoveMask;

            state.occupancy[White] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;

            state.mailBox[5] = int(Rook);
            state.mailBox[7] = 0;}

            state.pieceList.movePiece(5,7,White);

            break;
        case(58):
            {uint64_t rookMoveMask = (1ULL << 56) | (1ULL << 59);
            
            state.bitboards[Black][Rook] ^= rookMoveMask;
            
            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;

            state.mailBox[59] = int(Rook) + 8;
            state.mailBox[56] = 0;}

            state.pieceList.movePiece(59,56,Black);

            break;
        case(62):
            {uint64_t rookMoveMask = (1ULL << 61) | (1ULL << 63);

            state.bitboards[Black][Rook] ^= rookMoveMask;

            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;

            state.mailBox[61] = int(Rook) + 8;
            state.mailBox[63] = 0;}

            state.pieceList.movePiece(61,63,Black);

            break;
            
        default:
            std::runtime_error("Error, the king is making an illegal move");
    }
}


