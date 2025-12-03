#ifndef BOARD_H
#define BOARD_H

#include <vector>

#include "utils/Types.h"
    
class Board{

    private:
        void parseFenString(std::string fen);

    public:

        Board(const std::string fen, bool isAdversaryWhite);

        BoardState state;
        bool isAdversaryWhite;

        
        bool isAdversaryTurn();
        bool isPieceWhite(int pos);
        bool isSquareEmpty(int pos);
        bool isSquareAttacked(int pos, bool attackingColourIsWhite);
   
        uint64_t getFriendlyPieces(int pos);
        uint64_t getEnemyPieces(int pos);
        uint64_t getKingLocation(bool isWhite);
        uint64_t getPseudoLegalMoves(int pos);
        uint64_t applyLegalMoveValidation(const int pos, uint64_t moves);
        uint64_t getLegalMoves(const int pos);
        uint64_t getPromotionMoves(const int pos);
        uint64_t getAllAttacks(bool isWhite);
        uint64_t getAttacks(const int pos);
        uint64_t getPawnAttackers(int pos, bool attackingIsWhite);
        uint64_t getAttackers(int pos, bool attackingIsWhite);
        uint64_t pawnControlledSquare(bool controllingColourIsWhite);
        uint64_t getRay(int pos1, int pos2);
        uint64_t getPinnedPieces(bool isWhite, bool includeEnemies=false);
        uint64_t* getBitBoardFromPiece(int pieceEnum, bool isWhite);
        
        void makeMove(int from, int to, int promotionPieceType);
        void updatePieceBitBoards();

        int getPieceEnum(int pos);
        int getFirstBlocker(int pos,RaysDirection direction);
                        
};

enum Pieces {Pawn,Rook,Bishop,Knight,Queen,King};




#endif
