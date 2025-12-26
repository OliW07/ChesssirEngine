#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "attackHandler.h"

class Board;
class Game;

struct MoveList {
    Move moves[256];

    //Starting the count at one, as we always leave index 1 free for the bestMove
    int count = 1;

    MoveList(){
        moves[0].nullMove = true;
    }

    inline void add(const Move &move){
        if(count < 256){
            moves[count++] = move;
        }
    }

    inline void setBestMove(const Move bestMove){

        moves[0] = bestMove;
        //We don't have to store old moves[0] and replace, as either it was null or it was a previous best move, which was 
        //already a duplicate in the list, this ensures we only have one duplicate best move, which the TT will filter 
        //out so we don't have to search the move again
        
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
