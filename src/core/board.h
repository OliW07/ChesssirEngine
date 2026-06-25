#pragma once

#include <cstdint>

#include "Types.h"
#include "attackHandler.h"
#include "engine.h"
#include "moveGenerator.h"

struct SavedData {
    int halfMoveClock;
    int enPassantLoc;
    uint8_t castlingRights;
    Pieces capturedPiece;
    bool enPassantCapture;
    bool promotion;
    uint64_t zhash;
};

class Board {
    public:
    Board() { init(); };
    BoardState state;
    bool enginePlaysWhite;
    int historyIndex = 0;
    int eval = 0;

    bool isSquareEmpty(Square square);
    bool isCheck(Colour kingColour);
    bool isCapture(Move& move);

    Colour getColour(Square square);

    uint64_t getFriendlyPieces(Square square);
    uint64_t getEnemyPieces(Square square);
    uint64_t getKingLocation(Colour colour);
    uint64_t* getBitBoardFromPiece(int pieceEnum, bool isWhite);
    uint64_t getRay(Square square1, Square square2);

    void init();
    void resetPosition();
    void makeMove(Move move);
    void unmakeMove(Move move);
    void unmakeRookCastle(Move move);
    void handleCapture(int from, int to, bool isWhite);
    void handleEnpassant(int from, int to, bool isWhite);
    void handlePawnMove(int from, int to, bool isWhite, Pieces promotionPiece, uint64_t& pawnBitBoard);
    void handleRookCastle(int newKingLoc);
    void updateCastlingRights(int from, bool isWhite, Pieces pieceType);
    void saveHistory();

    int scoreMove(Move& move);
    int getPieceEnum(Square square);
    int getFirstBlocker(Square square, RaysDirection direction);

    SavedData history[2048] = {};
};

class Game {
    public:
    Board board;
    int ply = 0;
    MoveGenerator moveGenerator;
    AttackHandler attackHandler;
    Engine chesssir;

    Game() : moveGenerator(*this), attackHandler(*this), chesssir(*this) {};

    SearchInfo info;

    void setPosition(std::string fen, MoveList moves);
    bool isDraw();
    bool isTwoFoldRepition();
    bool isInsufficientMaterial();
    bool isFiftyMoveLimit();
};
