#ifndef BOARD_H
#define BOARD_h

#include <vector>

#include "utils/Types.h"
#include "data/precompute.h"


    
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
   
        uint64_t friendlyPieces(int pos);
        uint64_t getPseudoLegalMoves(int pos);
        uint64_t allAttacks(bool isWhite);
        uint64_t getAttacks(const int pos);
        
        void makeMove(int from, int to, int promotionPiece);
        void updatePieceBitBoards();

        int getPieceEnum(int pos);
                        
};

enum Pieces {Pawn,Rook,Bishop,Knight,Queen,King};




#endif