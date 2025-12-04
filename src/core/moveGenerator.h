#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "attackHandler.h"
#include "utils/Types.h"
#include "board.h"


class MoveGenerator {

    public:
       
        
        Board &board;
        AttackHandler attackHandler;

        MoveGenerator(Board &boardInstance) : board(boardInstance), attackHandler(boardInstance){
            
        }
            
        uint64_t getPseudoLegalMoves(int pos);
        uint64_t applyLegalMoveValidation(const int pos, uint64_t moves);
        uint64_t getLegalMoves(const int pos);
        uint64_t getPromotionMoves(const int pos);


};

#endif
