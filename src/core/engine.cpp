#include <random>

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

            
        
        Move move;
        move.from = pieceLoc;
        
        //Evaluate each legal move
        while(legalMoves)  {
            
            move.to = __builtin_ctzll(legalMoves);

            legalMoves &= (legalMoves - 1);
        
            boardInstance.makeMove(move);

            int eval = EvaluateState(boardInstance.state);
            if(eval > bestScore){
                bestScore = eval;
                bestMove = move;
            }

            boardInstance.unmakeMove(move);
            
        }

        while(promotionMoves){
            
            move.to = __builtin_ctzll(promotionMoves);
            promotionMoves &= (promotionMoves - 1);

            for(Pieces promotionPiece : {Rook,Queen,Bishop,Knight}){

                move.promotionPiece = promotionPiece;
                
                boardInstance.makeMove(move);

                int eval = EvaluateState(boardInstance.state);

                if(eval > bestScore){
                    bestScore = eval;
                    bestMove = move;
                }

                boardInstance.unmakeMove(move);
            }
            
        }
    }

    return bestMove;

}


int Engine::EvaluateState(BoardState &state){
    
    std::random_device rd; // Obtain a seed from the hardware
    std::mt19937 gen(rd()); 
    
    std::uniform_int_distribution<> distrib(1, 100);
    return distrib(gen);
}
