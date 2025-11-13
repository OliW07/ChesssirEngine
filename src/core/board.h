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
        uint64_t friendlyPieces(int pos);
        uint64_t getPseudoLegalMoves(int pos);
        void makeMove(int from, int to, int promotionPiece);
        void updatePieceBitBoards();

        int getPieceEnum(int pos);
                        
};

enum Pieces {Pawn,Rook,Bishop,Knight,Queen,King};




#endif