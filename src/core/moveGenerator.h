#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "attackHandler.h"
#include "utils/Types.h"
#include "board.h"

struct MoveList {
    Move moves[256];
    int count = 0;

    inline void add(const Move &move){

        moves[count++] = move;
    }

    Move *begin() { return moves;};
    Move *end() { return moves + count;};
};

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

        MoveList getAllMoves();


};


#endif
