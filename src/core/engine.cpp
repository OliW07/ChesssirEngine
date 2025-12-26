#include <chrono>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "moveGenerator.h"
#include "utils/Types.h"
#include "utils/log.h"
#include "engine.h"
#include "evaluate.h"

Move Engine::search(){

    startTime = std::chrono::steady_clock::now();
    game.board.isAdversaryWhite = !game.board.state.whiteToMove;
    
    setTimeToThink();
    
    Colours activeColour = (Colours)game.board.state.whiteToMove;

    Move overallBestMove = {} ;
    nodesVisited = 0; // Reset nodes at the start of a search

    int depth = 1;

    MoveList moves = game.moveGenerator.getAllMoves();
    if (moves.count == 0) {
        return {}; // Return a null move if there are no legal moves
    }
    overallBestMove = moves.moves[0]; // Default to the first legal move

    
    while(!abortSearch() && (game.info.depth == -1 || depth <= game.info.depth)){


        int bestScoreThisIteration = activeColour ? -2000000000 : 2000000000;
        Move bestMoveThisIteration = moves.moves[0]; // Default to a valid move

        int alpha = -999999999,
            beta  =  999999999;

        for(auto &move : moves){
           
            game.board.makeMove(move);
            int eval = miniMax(depth,alpha,beta);

            if(abortSearch()){
                game.board.unmakeMove(move);
                break;
            }
            game.board.unmakeMove(move);

            bool isBetter = activeColour ? (eval > bestScoreThisIteration) : (eval < bestScoreThisIteration);
            if(isBetter){
                bestScoreThisIteration = eval;
                bestMoveThisIteration = move;
            }

            if(activeColour && (eval>alpha)) alpha = eval;
            if(!activeColour && (eval<beta)) beta = eval;
            
            bool isPruning = activeColour ? (eval >= beta) : (eval <= alpha);
            if(isPruning){
                bestMoveThisIteration = move;
                break;
            }

        }

        if (!abortSearch()) {
            overallBestMove = bestMoveThisIteration;
            moves.setBestMove(overallBestMove);

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            long long nps = (elapsed > 0) ? (nodesVisited * 1000 / elapsed) : 0;

            std::stringstream info;
            info << "info depth " << depth 
                 << " score cp " << bestScoreThisIteration
                 << " nodes " << nodesVisited
                 << " nps " << nps
                 << " pv " << convertMoveToAlgebraicNotation(overallBestMove);
            
            log_uci(info.str(), uci_mutex);
        }

        depth++;
    }

    return overallBestMove;
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

int Engine::miniMax(int maxDepth, int alpha, int beta){
    
   nodesVisited++;

    if((nodesVisited & 2048) == 0 && abortSearch()) return 0; //Discard value later
    
    // Removed old, inaccurate logging
        
    if(maxDepth == 0) return evaluateState(game.board);

    Colours activeColour = (Colours)game.board.state.whiteToMove;
    int bestScore = activeColour ? -99999999 : 99999999;
    
 
    MoveList moves = game.moveGenerator.getAllMoves();

    if(moves.count == 0){

        if(game.attackHandler.isSquareAttacked(
                game.board.getKingLocation(activeColour), !activeColour)){
            //Checkmate
            return activeColour ? (-99999999 - maxDepth) : (99999999 + maxDepth);
        }
        //Stalemate
        return 0;
    }

    //Check for special draws
    if(game.isDraw()) return 0;

    for(auto &move : moves){
       
        game.board.makeMove(move);

        int eval = miniMax(maxDepth-1,alpha,beta);

        game.board.unmakeMove(move);

        bool isBetter = activeColour ? (eval > bestScore) : (eval < bestScore);
        if(isBetter) bestScore = eval;

        if(activeColour && (eval>alpha)) alpha = eval;
        if(!activeColour && (eval<beta)) beta = eval;
        
        bool isPruning = activeColour ? (eval >= beta) : (eval <= alpha);
        if(isPruning) return bestScore;
        
    }
    
    return bestScore;
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
    
    int myTime = game.board.isAdversaryWhite ? game.info.wtime : game.info.btime;
    int myInc  = game.board.isAdversaryWhite ? game.info.winc  : game.info.binc;

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
