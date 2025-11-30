#include <iostream>
#include <cctype>
#include <cmath>


#include "board.h"
#include "data/precompute.h"
#include "debug.h"
#include "utils/Types.h"
#include "tests/perft.h"


void init(Board &boardInstance);
void gameLoop(Board &boardInstance);



bool isPlaying = true;


int main(){

    const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const std::string CUSTOM_TEST_POSITION = "r1bqkbnr/pppppppp/8/8/3n4/4P3/PPPPKPPP/RNBQ1BNR w kq - 0 1";

    Board playingBoard(STARTING_FEN,true);

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
                
                int perftNodes = perftSearch(boardInstance);
                std::cout << perftNodes << "\n";

                for(const auto &[move,count] : moveBreakDown){
                    std::cout << move << " : " << count << "\n";
                }

                break;


                uint64_t moves = boardInstance.getLegalMoves(20);
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





