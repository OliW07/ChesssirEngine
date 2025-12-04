#ifndef BOARD_H
#define BOARD_H

#include <vector>

#include "utils/Types.h"
    
class Board{

    public:

        Board(const std::string fen, bool isAdversaryWhite);

        BoardState state;
        bool isAdversaryWhite;

        
        bool isAdversaryTurn();
        bool isPieceWhite(int pos);
        bool isSquareEmpty(int pos);
   
        uint64_t getFriendlyPieces(int pos);
        uint64_t getEnemyPieces(int pos);
        uint64_t getKingLocation(bool isWhite);
        uint64_t* getBitBoardFromPiece(int pieceEnum, bool isWhite);
        uint64_t getRay(int pos1, int pos2);

        void makeMove(int from, int to, int promotionPieceType);

        int getPieceEnum(int pos);
        int getFirstBlocker(int pos,RaysDirection direction);
                        
};

enum Pieces {Pawn,Rook,Bishop,Knight,Queen,King};




#endif
