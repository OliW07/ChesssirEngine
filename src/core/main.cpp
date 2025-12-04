#include <iostream>
#include <cctype>
#include <cmath>


#include "board.h"
#include "data/precompute.h"
#include "debug.h"
#include "utils/Types.h"
#include "tests/perft.h"
#include "moveGenerator.h"

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

    while(isPlaying){
        
        
        if(boardInstance.isAdversaryTurn()){

                std::string adversaryInput;
                uint64_t perftNodes = perftSearch(boardInstance);
                std::cout << perftNodes << "\n";

                for(const auto &[move,count] : moveBreakDown){
                    std::cout << move << " : " << count << "\n";
                }

                break;

                
                MoveGenerator moveGenerator(boardInstance);
                uint64_t moves = moveGenerator.getLegalMoves(33);
                std::cout << "Enter your move: ";
                std::cin >> adversaryInput;

                std::cout << "\n";
                
                std::vector<int> moveData = convertAlgebraicNotationToMoves(adversaryInput);
               
                if(moveData.empty()){

                    //Bad user input
                    std::cout << "Error! Invalid algebraic notation, please try again \n";
                    continue;
                }
                
                boardInstance.makeMove(moveData[0],moveData[1],moveData[2]);
                visualiseGraphicBoard(boardInstance.state);
                //Move the piece on the board

               break; 


        }else{
             
        }
        
        
    }


}





