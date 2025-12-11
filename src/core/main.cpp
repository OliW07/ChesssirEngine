#include <iostream>
#include <cctype>
#include <cmath>


#include "board.h"
#include "data/precompute.h"
#include "debug.h"
#include "evaluate.h"
#include "utils/Types.h"
#include "tests/perft.h"
#include "moveGenerator.h"
#include "engine.h"


void init(Board &boardInstance);
void gameLoop(Board &boardInstance);



bool isPlaying = true;


int main(){

    const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const std::string CUSTOM_TEST_POSITION = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";


    Board playingBoard(CUSTOM_TEST_POSITION,true);
    init(playingBoard);

    

    visualiseGraphicBoard(playingBoard.state);

    gameLoop(playingBoard);

    return 0;
}

void init(Board &boardInstance){

    std::cout << "Initialising engine... \n";
    
    precomputeBitBoardMoves();
}

void gameLoop(Board &boardInstance){


    std::cout << "Chess Game Starting... \n";

    Engine chesssirEngine;
    while(isPlaying){
        
        
        if(boardInstance.isAdversaryTurn()){

                std::string adversaryInput;
        /*        uint64_t perftNodes = perftSearch(boardInstance);
                std::cout << perftNodes << "\n";

                for(const auto &[move,count] : moveBreakDown){
                    std::cout << move << " : " << count << "\n";
                }

                break;
       */
                
                int eval = evaluateState(boardInstance);
                std::cout << "Evaluation of position: " ;
                std::cout << eval << '\n';
                std::cout << "Enter your move or type q to quit: ";
                std::cin >> adversaryInput;

                std::cout << "\n";
                
                if(adversaryInput == "q"){
                    isPlaying = false;
                    break;
                } 

                Move playerMove = convertAlgebraicNotationToMove(adversaryInput);
               
                if(playerMove.nullMove){

                    //Bad user input
                    std::cout << "Error! Invalid algebraic notation, please try again \n";
                    continue;
                }
                
                boardInstance.makeMove(playerMove);
                visualiseGraphicBoard(boardInstance.state);
                //Move the piece on the board



        }else{
            
            Move bestMove = chesssirEngine.bestMove(boardInstance);
            
            std::string notation = convertMoveToAlgebraicNotation(bestMove);
           
            std::cout << "Computer move: ";
            std::cout << notation << '\n';

            boardInstance.makeMove(bestMove);
            visualiseGraphicBoard(boardInstance.state);

        }
        
        
    }


}





