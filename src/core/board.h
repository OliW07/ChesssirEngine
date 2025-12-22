#ifndef BOARD_H
#define BOARD_H

#include <vector>

#include "attackHandler.h"
#include "utils/Types.h"
#include "moveGenerator.h"
#include "engine.h"
    
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
        
        Board(){init();};
        BoardState state;
        bool isAdversaryWhite;
        int historyIndex = 0;

        bool isAdversaryTurn();
        bool isPieceWhite(int pos);
        bool isSquareEmpty(int pos);
        bool isCheck(Colours kingColour);
   
        uint64_t getFriendlyPieces(int pos);
        uint64_t getEnemyPieces(int pos);
        uint64_t getKingLocation(bool isWhite);
        uint64_t* getBitBoardFromPiece(int pieceEnum, bool isWhite);
        uint64_t getRay(int pos1, int pos2);

        void init();
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


class Game {
    
    public:
        Board board;
        MoveGenerator moveGenerator;
        AttackHandler attackHandler;
        Engine chesssir;

        Game() : moveGenerator(*this), attackHandler(*this), chesssir(*this) {};

        SearchInfo info;

        void setPosition(std::string fen, MoveList moves);
        bool isDraw();
        bool isThreeFoldRepition();
        bool isInsufficientMaterial();
        bool isFiftyMoveLimit();
};


#endif
