#include "moveGenerator.h"
#include "utils/Types.h"
#include <cmath>
#include "engine.h"

Move Engine::bestMove(Board &boardInstance){
    MoveGenerator moveGenerator(boardInstance);
    
    Colours activeColour = (Colours)boardInstance.state.whiteToMove;
    int bestScore = -INFINITY;
    Move bestMove;

    for(int i = 0; i < boardInstance.state.pieceList.pieceCount[activeColour]; i++){
        
        int pieceLoc = boardInstance.state.pieceList.list[activeColour][i];
        uint64_t legalMoves = moveGenerator.getLegalMoves(pieceLoc);
        uint64_t promotionMoves = moveGenerator.getPromotionMoves(pieceLoc);

            
        
        Move move; move.from = pieceLoc;
        
        //Evaluate each legal move
        while(legalMoves)  {
            
            move.to = __builtin_ctzll(legalMoves);

            legalMoves &= (legalMoves - 1);
        
            boardInstance.makeMove(move);

            int eval = EvaluateState(&boardInstance.state);
            if(eval > bestScore){
                bestScore = eval;
                bestMove = move;
            }

            boardInstance.unmakeMove(move);
            
        }

        while(promotionMoves){

            
        }
    }

}
