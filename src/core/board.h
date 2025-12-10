#ifndef BOARD_H
#define BOARD_H

#include <vector>

#include "utils/Types.h"
    
struct SavedData {

    int halfMoveClock;
    int enPassantLoc;
    uint8_t castlingRights;
    Pieces capturedPiece;
    bool enPassantCapture;
    bool promotion;

};

class Board{

    public:

        Board(const std::string fen, bool isAdversaryWhite);

        BoardState state;
        bool isAdversaryWhite;
        int historyIndex;
        
        bool isAdversaryTurn();
        bool isPieceWhite(int pos);
        bool isSquareEmpty(int pos);
   
        uint64_t getFriendlyPieces(int pos);
        uint64_t getEnemyPieces(int pos);
        uint64_t getKingLocation(bool isWhite);
        uint64_t* getBitBoardFromPiece(int pieceEnum, bool isWhite);
        uint64_t getRay(int pos1, int pos2);

        void makeMove(Move move);
        void unmakeMove(Move move);
        void unmakeRookCastle(Move move);
        void handleCapture(int from, int to, bool isWhite);
        void handleEnpassant(int from, int to, bool isWhite);
        void handlePawnMove(int from, int to, bool isWhite, Pieces promotionPiece, uint64_t &pawnBitBoard);
        void handleRookCastle(int newKingLoc);
        void updateCastlingRights(int from, bool isWhite, Pieces pieceType);
        void saveHistory();

        int getPieceEnum(int pos);
        int getFirstBlocker(int pos,RaysDirection direction);
    

        SavedData history[2048] = {};
};




#endif
