#include <chrono>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "moveGenerator.h"
#include "tranpositionTable.h"
#include "utils/Types.h"
#include "utils/log.h"
#include "engine.h"
#include "board.h"
#include "evaluate.h"
#include "debug.h"

Move Engine::search(){

    startTime = std::chrono::steady_clock::now();
    // enginePlaysWhite is set in setPosition() based on initial position
    
    setTimeToThink();
    
    Colours activeColour = (Colours)game.board.state.whiteToMove;

    nodesVisited = 0; 

    int maxDepth = (game.info.depth == -1) ? 40 : game.info.depth;

    int alpha = -999999999,
        beta  =  999999999;

    TTEntry entry;

    for(int currentDepth = 1; (currentDepth <= maxDepth) && !abortSearch(); currentDepth++){


        int eval = -negamax(currentDepth, alpha, beta, game.ply + 1);


        //Write the current depth search to the entry
        bool foundInTT = tt.probe(game.board.state.zhash,entry);

        int absoluteEval = game.board.state.whiteToMove ? entry.eval : -entry.eval;
        log_uci(currentDepth, absoluteEval, nodesVisited, unpackMove(entry.bestMove), startTime, uci_mutex);


        //Found a forced mate, no point trying to find the best move
        if(std::abs(eval) > MATESCORE - 1000) break;

    }

    //The deepest search that finished was the last to write to the TT
    return unpackMove(entry.bestMove);

}
int Engine::negamax(int maxDepth, int alpha, int beta, int ply){
    
    TTEntry stored;
    bool foundInTT = tt.probe(game.board.state.zhash,stored);
    if(foundInTT){
    
        if(stored.eval >= MATESCORE-1000){
            stored.eval -= ply;
        }else if(stored.eval <= -MATESCORE+1000){
            stored.eval += ply;
        }
        if(stored.depth >= maxDepth){

            if(stored.type == NodeType::Exact) return stored.eval;
            if(stored.type == NodeType::Lowerbound && stored.eval  >= beta) return stored.eval;
            if(stored.type == NodeType::Upperbound && stored.eval <= alpha) return stored.eval;
        }
    }
    nodesVisited++;

    if((nodesVisited & 2048) == 0 && abortSearch()) return 0; //Discard value later
    
    if(maxDepth == 0) return game.board.state.whiteToMove ? game.board.eval : -game.board.eval;

    Colours activeColour = (Colours)game.board.state.whiteToMove;
    int bestScore = -999999;
    int alphaOrig = alpha;
    Move bestMoveThisNode = {};
    NodeType type = NodeType::Exact;

    MoveList moves = game.moveGenerator.getAllMoves();

    //If the stored best move is not null
    if (foundInTT && stored.bestMove != 0 && !((stored.bestMove >> 15) & 1)) { 
        moves.setBestMove(unpackMove(stored.bestMove)); 
    }

    if(moves.count == 0){

        bool inCheck = game.attackHandler.isSquareAttacked(game.board.getKingLocation(activeColour), !activeColour);
        return inCheck ? (-MATESCORE + ply) : 0;
    }

    //Check for special draws
    if(game.isDraw()) return 0;

    for(int i = 0; i < moves.count; i++){
        
        moves.sortNext(i);
        Move move = moves.moves[i];

        game.board.makeMove(move);

        int eval = -negamax(maxDepth-1,-beta,-alpha,ply+1);

        game.board.unmakeMove(move);

        if(eval > bestScore){
            bestScore = eval;
            bestMoveThisNode = move;
        }

        if(eval >= beta){
            type = NodeType::Lowerbound;
            break;
        }

        if(eval > alpha){
            alpha = eval;
        }
         
    }
    int scoreToStore = bestScore;
    if (scoreToStore > MATESCORE-1000) scoreToStore += ply;
    if (scoreToStore < -MATESCORE+1000) scoreToStore -= ply;


    //Update TT with searched results
    if(bestScore <= alphaOrig) type = NodeType::Upperbound;

    tt.write(game.board.state.zhash,maxDepth,ply,scoreToStore,bestMoveThisNode,type);

    return bestScore;
}


void Engine::writeBestMove(){
    std::ofstream df("engine_log.txt", std::ios::app);
    df << "--- Thread Active ---" << std::endl;

    log_uci("info string Chessir is calculating...", uci_mutex);

    Move bestMove = search(); 
    std::string moveStr = convertMoveToAlgebraicNotation(bestMove);

    df << "Search finished. Resulting Move: [" << moveStr << "]" << std::endl;

    if (!moveStr.empty()) {
        log_uci("bestmove " + moveStr, uci_mutex);
    }

    df << "Confirmed sent to GUI: bestmove " << moveStr << std::endl;
    df.close();
}


bool Engine::abortSearch(){
     
    if(stopRequested) return true;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

    if((elapsed >= timeToThink) && game.info.depth == -1) return true;

    return false;
}

void Engine::setTimeToThink(){

    //Check if we have used our quota of time for this search
    
    int myTime = game.board.enginePlaysWhite ? game.info.wtime : game.info.btime;
    int myInc  = game.board.enginePlaysWhite ? game.info.winc  : game.info.binc;

    if (game.info.movetime != -1) {
        // EXACT time per move
        timeToThink = game.info.movetime; 
    }
    else if (game.info.infinite) {
        // Analyze until user types "stop"
        timeToThink = 99999999; 
    }
    else if (myTime != -1) {

        if (game.info.movestogo != -1) {
            // Tournament mode
            timeToThink = myTime / game.info.movestogo; 
            // safety buffer
            if (timeToThink > 50) timeToThink -= 50; 
        } else {
            //Standard mode 
            timeToThink = (myTime / 20) + (myInc / 2);
        }
    } 
    else if (game.info.depth != -1) {
        //Handle in search: Set to huge time so abortSearch relies on depth check
        timeToThink = 99999999;
    }
    else {
        // Fallback default
        timeToThink = 1000;
    }

}
