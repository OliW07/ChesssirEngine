#include "moveGenerator.h"
#include "utils/Types.h"
#include <cmath>
#include "engine.h"
#include "evaluate.h"

Move Engine::bestMove(Board &boardInstance, int maxDepth){
    MoveGenerator moveGenerator(boardInstance);
    
    Colours activeColour = (Colours)boardInstance.state.whiteToMove;
    int bestScore = activeColour ? -99999999 : 99999999;
    Move bestMove = {true,-1,-1,None};

    MoveList moves = moveGenerator.getAllMoves();

    for(auto &move : moves){
       
        boardInstance.makeMove(move);
        int eval = miniMax(boardInstance,maxDepth);
        boardInstance.unmakeMove(move);

        bool isBetter = activeColour ? (eval > bestScore) : (eval < bestScore);

        if(isBetter){
            bestScore = eval;
            bestMove = move;
        }
    }

    return bestMove;
}

int Engine::miniMax(Board &boardInstance, int maxDepth){
    
    if(maxDepth == 0) return evaluateState(boardInstance);

    MoveGenerator moveGenerator(boardInstance);
    Colours activeColour = (Colours)boardInstance.state.whiteToMove;
    int bestScore = activeColour ? -99999999 : 99999999;
    

 
    MoveList moves = moveGenerator.getAllMoves();

    if(moves.count == 0){

        if(moveGenerator.attackHandler.isSquareAttacked(
                boardInstance.getKingLocation(activeColour), !activeColour)){
            //Checkmate
            return activeColour ? (-99999999 - maxDepth) : (99999999 + maxDepth);
        }
        //Stalemate
        return 0;
    }

    for(auto &move : moves){
       
        boardInstance.makeMove(move);

        int eval = miniMax(boardInstance,maxDepth-1);

        boardInstance.unmakeMove(move);

        bool isBetter = activeColour ? (eval > bestScore) : (eval < bestScore);
        if(isBetter) bestScore = eval;

        
    }
    
    return bestScore;
}
