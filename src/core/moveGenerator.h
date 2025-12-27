#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "attackHandler.h"

class Board;
class Game;

struct MoveList {
    Move moves[256];
    int count;

    MoveList(){
        moves[0].nullMove = true;
        count = 1;
    }

    inline void add(const Move &move){
        if(count < 256){
            moves[count++] = move;
        }
    }

    inline void setBestMove(const Move bestMove){

        for(int i = 1; i < count; i++){
            if(moves[i] == bestMove){
                moves[0] = moves[i];
                moves[i] = moves[count-1];
                //Slot the best move into the empty slot so decrease count as null move is gone
                count--;
                break;
            }
        }
    }

    Move *begin() { 

        //If the first index (best move slot) is empty, skip it
        if(moves[0].nullMove) return moves + 1;
        else return moves;
    };
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
