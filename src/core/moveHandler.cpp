#include "board.h"
#include <iostream>

void Board::makeMove(Move move){

    history[historyIndex].halfMoveClock = state.halfMoveClock;
    history[historyIndex].enPassantLoc = state.enPassantSquare;
    history[historyIndex].castlingRights = state.castlingRights;
    history[historyIndex].capturedPiece = None;
    history[historyIndex].enPassantCapture = false;
    history[historyIndex].promotion = false;
    
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

    historyIndex++;
     
    
}

void Board::unmakeMove(Move move){

    state.whiteToMove = !state.whiteToMove;

    historyIndex--;

    
    SavedData restored = history[historyIndex];

    Pieces pieceType = restored.promotion ? Pawn : (Pieces)getPieceEnum(move.to);
    bool isWhite = state.whiteToMove;

    uint64_t *pieceBitBoard = getBitBoardFromPiece((int)pieceType,isWhite);

    if(restored.promotion){

        //We need to replace the phantom pawn as when we XOR pawn bitboard later with no pawn there, one appears
        *pieceBitBoard ^= (1ULL << move.to);

        //Remove the promoted piece
        Pieces promotionType = (Pieces)getPieceEnum(move.to);
        uint64_t *promotedPieceBitBoard = getBitBoardFromPiece(promotionType,isWhite);
        *promotedPieceBitBoard ^= (1ULL << move.to);
        
    }
    
    state.castlingRights = restored.castlingRights;
    state.enPassantSquare = restored.enPassantLoc;
    state.halfMoveClock = restored.halfMoveClock;
    
    if(pieceType == King && (abs(convertLocationToColumns(move.to) - convertLocationToColumns(move.from)) > 1)){
        unmakeRookCastle(move);
        
    }

    uint64_t moveMask = (1ULL << move.to) | (1ULL << move.from);

    *pieceBitBoard ^= moveMask;

    state.mailBox[move.from] = convertPieceToBinary(pieceType, isWhite);
    state.mailBox[move.to] = 0;

    state.pieceList.movePiece(move.from,move.to,(Colours)isWhite);

    state.occupancy[isWhite] ^= moveMask;
    state.occupancy[Both] ^= moveMask;

    if(restored.enPassantCapture){
        int capturedPawnPos = isWhite ? move.to - 8 : move.to + 8;
        //Add the pawn back
        uint64_t *capturedBitBoard = getBitBoardFromPiece(Pawn,!isWhite);
        *capturedBitBoard ^= (1ULL << capturedPawnPos);

        state.occupancy[!isWhite] ^= (1ULL << capturedPawnPos);
        state.occupancy[Both] ^= (1ULL << capturedPawnPos);
        
        state.mailBox[capturedPawnPos] = convertPieceToBinary(Pawn, !isWhite);
        state.pieceList.addPiece(capturedPawnPos, (Colours)!isWhite);

    }else if(restored.capturedPiece != None){
        uint64_t *capturedBitBoard = getBitBoardFromPiece(restored.capturedPiece,!isWhite);
        *capturedBitBoard ^= (1ULL << move.to);

        state.occupancy[!isWhite] ^= (1ULL << move.to);
        state.occupancy[Both] ^= (1ULL << move.to);

        state.mailBox[move.to] = convertPieceToBinary(restored.capturedPiece, !isWhite);
        state.pieceList.addPiece(move.to, (Colours)!isWhite);
    }


    if(state.whiteToMove != state.whiteStarts) state.fullMoveClock--;

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

    history[historyIndex].capturedPiece = capturedPiece;
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
    history[historyIndex].capturedPiece = Pawn;
    history[historyIndex].enPassantCapture = true;
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
            
            history[historyIndex].promotion = true;
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

void Board::unmakeRookCastle(Move move){

    switch(move.to){
        case(2):
            {uint64_t rookMoveMask = (1ULL << 0) | (1ULL << 3);
            state.bitboards[White][Rook] ^= rookMoveMask;

            state.occupancy[White] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            
            state.mailBox[0] = int(Rook);
            state.mailBox[3] = 0;}

            state.pieceList.movePiece(0,3,White);

            break;
        case(6):
            {uint64_t rookMoveMask = (1ULL << 5) | (1ULL << 7);
            state.bitboards[White][Rook] ^= rookMoveMask;

            state.occupancy[White] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;

            state.mailBox[7] = int(Rook);
            state.mailBox[5] = 0;}

            state.pieceList.movePiece(7,5,White);

            break;
        case(58):
            {uint64_t rookMoveMask = (1ULL << 56) | (1ULL << 59);
            
            state.bitboards[Black][Rook] ^= rookMoveMask;
            
            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;

            state.mailBox[56] = int(Rook) + 8;
            state.mailBox[59] = 0;}

            state.pieceList.movePiece(56,59,Black);

            break;
        case(62):
            {uint64_t rookMoveMask = (1ULL << 61) | (1ULL << 63);

            state.bitboards[Black][Rook] ^= rookMoveMask;

            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;

            state.mailBox[63] = int(Rook) + 8;
            state.mailBox[61] = 0;}

            state.pieceList.movePiece(63,61,Black);

            break;
            
        default:
            std::runtime_error("Error, the king is making an illegal move");
    }

}


