#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "attackHandler.h"

class Board;
class Game;

struct MoveList {
    Move moves[256];
    int count = 0;

    inline void add(const Move &move){
        if(count < 256){
            moves[count++] = move;
        }
    }

    Move *begin() { return moves;};
    Move *end() { return moves + count;};
};

class MoveGenerator {


    private:
        
        Game &game;
    public:

        MoveGenerator(Game &gameParam) : game(gameParam){
            
        }
            
        uint64_t getPseudoLegalMoves(int pos);
        uint64_t applyLegalMoveValidation(const int pos, uint64_t moves);
        uint64_t getLegalMoves(const int pos);
        uint64_t getPromotionMoves(const int pos);

        MoveList getAllMoves();


};


#endif
