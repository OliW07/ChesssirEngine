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
        bool isWhite(int pos);
        bool friendlyPieces(int pos);
        uint64_t getPseduoLegalMoves(int pos);
        void makeMove(int from, int to, int promotionPiece);
                        
};

#endif