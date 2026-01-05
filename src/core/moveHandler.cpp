#include "Types.h"
#include "board.h"
#include "zobrist.h"
#include "evaluate.h"

#include <cstdint>
#include <iostream>

extern ZobristKeys Zobrist;

void Board::makeMove(Move move){
    if(move.nullMove) return;


    history[historyIndex].halfMoveClock = state.halfMoveClock;
    history[historyIndex].enPassantLoc = state.enPassantSquare;
    history[historyIndex].castlingRights = state.castlingRights;
    history[historyIndex].capturedPiece = None;
    history[historyIndex].enPassantCapture = false;
    history[historyIndex].promotion = false;
    history[historyIndex].zhash = state.zhash;

    state.zhash ^= Zobrist.sideKey;
    if(state.enPassantSquare != -1) state.zhash ^= Zobrist.enPassantKeys[convertLocationToColumns(state.enPassantSquare)];

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
    state.bitboards[Both][pieceType] ^= moveMask; 

    state.mailBox[move.to] = convertPieceToBinary(pieceType, isWhite);
    state.mailBox[move.from] = 0;

    state.pieceList.movePiece(move.to,move.from,(Colours)isWhite);

    state.zhash ^= Zobrist.pieceKeys[isWhite][pieceType][move.to];
    state.zhash ^= Zobrist.pieceKeys[isWhite][pieceType][move.from];


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
    state.zhash ^= Zobrist.sideKey;

    historyIndex--;

    
    SavedData restored = history[historyIndex];

    Pieces pieceType = restored.promotion ? Pawn : (Pieces)getPieceEnum(move.to);
    bool isWhite = state.whiteToMove;

    uint64_t *pieceBitBoard = getBitBoardFromPiece((int)pieceType,isWhite);

    if(restored.promotion){

        //We need to replace the phantom pawn as when we XOR pawn bitboard later with no pawn there, one appears

        uint64_t newPieceMask = (1ULL << move.to);
        *pieceBitBoard ^= newPieceMask;

        //Remove the promoted piece
        Pieces promotionType = (Pieces)getPieceEnum(move.to);
        uint64_t *promotedPieceBitBoard = getBitBoardFromPiece(promotionType,isWhite);
        *promotedPieceBitBoard ^= newPieceMask;
        state.bitboards[Both][promotionType] ^= newPieceMask;
        state.bitboards[Both][Pawn] ^= newPieceMask;
        state.zhash ^= Zobrist.pieceKeys[isWhite][Pawn][move.to];
        state.zhash ^= Zobrist.pieceKeys[isWhite][promotionType][move.to];

        eval -= isWhite ? PieceValues[promotionType] : -PieceValues[promotionType];
        eval += isWhite ? PieceValues[Pawn] : -PieceValues[Pawn];
    
        
    }
    //XOR old hash data out
    if(state.enPassantSquare != -1){
        state.zhash ^= Zobrist.enPassantKeys[convertLocationToColumns(state.enPassantSquare)];
    }
    state.zhash ^= Zobrist.castlingKeys[state.castlingRights];

    state.castlingRights = restored.castlingRights;
    state.enPassantSquare = restored.enPassantLoc;
    state.halfMoveClock = restored.halfMoveClock;

    //XOR new hash data in
    state.zhash ^= Zobrist.castlingKeys[restored.castlingRights];
    if(restored.enPassantLoc != -1){

        state.zhash ^= Zobrist.enPassantKeys[convertLocationToColumns(restored.enPassantLoc)];
    }
    
    if(pieceType == King && (abs(convertLocationToColumns(move.to) - convertLocationToColumns(move.from)) > 1)){
        unmakeRookCastle(move);
        
    }

    uint64_t moveMask = (1ULL << move.to) | (1ULL << move.from);

    *pieceBitBoard ^= moveMask;
    state.bitboards[Both][pieceType] ^= moveMask;

    state.mailBox[move.from] = convertPieceToBinary(pieceType, isWhite);
    state.mailBox[move.to] = 0;

    state.pieceList.movePiece(move.from,move.to,(Colours)isWhite);

    state.occupancy[isWhite] ^= moveMask;
    state.occupancy[Both] ^= moveMask;

    state.zhash ^= Zobrist.pieceKeys[isWhite][pieceType][move.to];
    state.zhash ^= Zobrist.pieceKeys[isWhite][pieceType][move.from];

    if(restored.enPassantCapture){
        int capturedPawnPos = isWhite ? move.to - 8 : move.to + 8;

        uint64_t pawnMask = (1ULL << capturedPawnPos);
        //Add the pawn back
        uint64_t *capturedBitBoard = getBitBoardFromPiece(Pawn,!isWhite);
        *capturedBitBoard ^= pawnMask;

        eval -= (isWhite ? PieceValues[Pawn] : -PieceValues[Pawn]);
        
        state.occupancy[!isWhite] ^= pawnMask;
        state.occupancy[Both] ^= pawnMask;

        state.bitboards[Both][Pawn] ^= pawnMask;
        state.zhash ^= Zobrist.pieceKeys[!isWhite][Pawn][capturedPawnPos];
        
        state.mailBox[capturedPawnPos] = convertPieceToBinary(Pawn, !isWhite);
        state.pieceList.addPiece(capturedPawnPos, (Colours)!isWhite);

    }else if(restored.capturedPiece != None){
        uint64_t *capturedBitBoard = getBitBoardFromPiece(restored.capturedPiece,!isWhite);
        uint64_t capturedMask = (1ULL << move.to);
        
        *capturedBitBoard ^= capturedMask;

        state.occupancy[!isWhite] ^= capturedMask;
        state.occupancy[Both] ^= capturedMask;

        state.bitboards[Both][restored.capturedPiece] ^= capturedMask;
        state.zhash ^= Zobrist.pieceKeys[!isWhite][restored.capturedPiece][move.to];

        state.mailBox[move.to] = convertPieceToBinary(restored.capturedPiece, !isWhite);
        state.pieceList.addPiece(move.to, (Colours)!isWhite);

        eval -= (isWhite ? PieceValues[restored.capturedPiece] : -PieceValues[restored.capturedPiece]);
    }


    if(state.whiteToMove != state.whiteStarts) state.fullMoveClock--;
    
}

void Board::handleCapture(int from, int to,bool isWhite){

    //Capturing a piece
    
    Pieces capturedPiece = (Pieces)getPieceEnum(to);

    if(capturedPiece == Rook && state.castlingRights > 0){

        //XOR the old castlingRights
        state.zhash ^= Zobrist.castlingKeys[state.castlingRights];
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


        //XOR the new castlingRights
        state.zhash ^= Zobrist.castlingKeys[state.castlingRights];
    }

    //Caputuring an enemy piece increases your sides eval
    eval += (isWhite ? PieceValues[capturedPiece] : -PieceValues[capturedPiece]);
    
    uint64_t *capturedBitBoard = getBitBoardFromPiece(capturedPiece,!isWhite);
    uint64_t capturedMask = (1ULL << to);

    //Toggle off the captured piece
    *capturedBitBoard ^= capturedMask;
    state.occupancy[!isWhite] ^= capturedMask;
    state.mailBox[to] = convertPieceToBinary(capturedPiece,isWhite);
    state.pieceList.removePiece(to,(Colours)!isWhite);

    state.bitboards[Both][capturedPiece] ^= capturedMask;

    state.zhash ^= Zobrist.pieceKeys[!isWhite][capturedPiece][to];

    state.halfMoveClock = -1;

    history[historyIndex].capturedPiece = capturedPiece;
}

void Board::handleEnpassant(int from, int to, bool isWhite){

    uint64_t *capturedBitBoard = getBitBoardFromPiece(Pawn,!isWhite);
    int capturePos = isWhite ? to - 8 : to + 8;
    uint64_t captureMask = 1ULL << capturePos;

    eval += isWhite ? PieceValues[Pawn] : -PieceValues[Pawn];

    *capturedBitBoard ^= captureMask;
    state.occupancy[!isWhite] ^= captureMask;
    state.occupancy[Both] ^= captureMask;
    state.bitboards[Both][Pawn] ^= captureMask;
    state.mailBox[capturePos] = 0;
    state.zhash ^= Zobrist.pieceKeys[!isWhite][Pawn][capturePos];
    state.pieceList.removePiece(capturePos,(Colours)!isWhite);
    state.halfMoveClock = -1;
    history[historyIndex].capturedPiece = Pawn;
    history[historyIndex].enPassantCapture = true;
}

void Board::updateCastlingRights(int from, bool isWhite, Pieces pieceType){

    uint8_t oldRights = state.castlingRights;
    bool changedRights = false;

    if(isWhite){

        if((pieceType == Rook && from == 0) || pieceType == King){
            state.castlingRights &= ~4;
            changedRights = true;
        }

        if((pieceType == Rook && from == 7) || pieceType == King){
            state.castlingRights &= ~8;
            changedRights = true;
        }


    }

    else{
        
        if((pieceType == Rook && from == 63) || pieceType == King){
            state.castlingRights &= ~2;
            changedRights = true;
        }

        if((pieceType == Rook && from == 56) || pieceType == King){
            state.castlingRights &= ~1;
            changedRights = true;
        }
    }

    if(changedRights){
        state.zhash ^= (Zobrist.castlingKeys[oldRights] ^ Zobrist.castlingKeys[state.castlingRights]);
    }

}

void Board::handlePawnMove(int from, int to, bool isWhite, Pieces promotionPiece, uint64_t &pawnBitBoard){

        if(abs(convertLocationToRows(from) - convertLocationToRows(to)) == 2){
            //Set the enpassant square
            state.enPassantSquare = isWhite ? from+8 : from-8;
            state.zhash ^= Zobrist.enPassantKeys[convertLocationToColumns(state.enPassantSquare)];
            
        }else{
            state.enPassantSquare = -1;
        }

        if((isWhite && convertLocationToRows(to) == 7) || (!isWhite && convertLocationToRows(to) == 0)){
            //Pawn promotes
            uint64_t *newPieceBitBoard = getBitBoardFromPiece(promotionPiece,isWhite);
            uint64_t newPieceMask = (1ULL << to);
            *newPieceBitBoard |= newPieceMask;
            pawnBitBoard ^= newPieceMask;
            state.bitboards[Both][promotionPiece] ^= newPieceMask;
            state.bitboards[Both][Pawn] ^= newPieceMask; 
            state.mailBox[to] = convertPieceToBinary(promotionPiece,isWhite);
            //Toggle on promotion piece hash, toggle off pawn hash, as the makeMove will toggle it on later, so this cancels out and ensures no pawn remains on the backrank.
            state.zhash ^= Zobrist.pieceKeys[isWhite][promotionPiece][to];
            state.zhash ^= Zobrist.pieceKeys[isWhite][Pawn][to]; 
            
            eval += isWhite ? PieceValues[promotionPiece] : -PieceValues[promotionPiece];
            eval -= isWhite ? PieceValues[Pawn] : -PieceValues[Pawn];

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
            state.bitboards[Both][Rook] ^= rookMoveMask;
            
            state.mailBox[3] = int(Rook);
            state.mailBox[0] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[White][Rook][0] ^ Zobrist.pieceKeys[White][Rook][3]);

            state.pieceList.movePiece(3,0,White);

            break;
        case(6):
            {uint64_t rookMoveMask = (1ULL << 5) | (1ULL << 7);
            state.bitboards[White][Rook] ^= rookMoveMask;

            state.occupancy[White] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            state.bitboards[Both][Rook] ^= rookMoveMask;

            state.mailBox[5] = int(Rook);
            state.mailBox[7] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[White][Rook][5] ^ Zobrist.pieceKeys[White][Rook][7]);

            state.pieceList.movePiece(5,7,White);

            break;
        case(58):
            {uint64_t rookMoveMask = (1ULL << 56) | (1ULL << 59);
            
            state.bitboards[Black][Rook] ^= rookMoveMask;
            
            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            state.bitboards[Both][Rook] ^= rookMoveMask;

            state.mailBox[59] = int(Rook) + 8;
            state.mailBox[56] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[Black][Rook][56] ^ Zobrist.pieceKeys[Black][Rook][59]);

            state.pieceList.movePiece(59,56,Black);

            break;
        case(62):
            {uint64_t rookMoveMask = (1ULL << 61) | (1ULL << 63);

            state.bitboards[Black][Rook] ^= rookMoveMask;

            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            state.bitboards[Both][Rook] ^= rookMoveMask;

            state.mailBox[61] = int(Rook) + 8;
            state.mailBox[63] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[Black][Rook][61] ^ Zobrist.pieceKeys[Black][Rook][63]);

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
            state.bitboards[Both][Rook] ^= rookMoveMask;
            
            state.mailBox[0] = int(Rook);
            state.mailBox[3] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[White][Rook][0] ^ Zobrist.pieceKeys[White][Rook][3]);

            state.pieceList.movePiece(0,3,White);

            break;
        case(6):
            {uint64_t rookMoveMask = (1ULL << 5) | (1ULL << 7);
            state.bitboards[White][Rook] ^= rookMoveMask;

            state.occupancy[White] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            state.bitboards[Both][Rook] ^= rookMoveMask;

            state.mailBox[7] = int(Rook);
            state.mailBox[5] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[White][Rook][5] ^ Zobrist.pieceKeys[White][Rook][7]);


            state.pieceList.movePiece(7,5,White);

            break;
        case(58):
            {uint64_t rookMoveMask = (1ULL << 56) | (1ULL << 59);
            
            state.bitboards[Black][Rook] ^= rookMoveMask;
            
            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            state.bitboards[Both][Rook] ^= rookMoveMask;

            state.mailBox[56] = int(Rook) + 8;
            state.mailBox[59] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[Black][Rook][56] ^ Zobrist.pieceKeys[Black][Rook][59]);

            state.pieceList.movePiece(56,59,Black);

            break;
        case(62):
            {uint64_t rookMoveMask = (1ULL << 61) | (1ULL << 63);

            state.bitboards[Black][Rook] ^= rookMoveMask;

            state.occupancy[Black] ^= rookMoveMask;
            state.occupancy[Both] ^= rookMoveMask;
            state.bitboards[Both][Rook] ^= rookMoveMask;

            state.mailBox[63] = int(Rook) + 8;
            state.mailBox[61] = 0;}

            state.zhash ^= (Zobrist.pieceKeys[Black][Rook][61] ^ Zobrist.pieceKeys[Black][Rook][63]);

            state.pieceList.movePiece(63,61,Black);

            break;
            
        default:
            std::runtime_error("Error, the king is making an illegal move");
    }

}


