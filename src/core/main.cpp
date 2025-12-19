#include <iostream>
#include <cctype>
#include <cmath>


#include "board.h"
#include "data/precompute.h"
#include "tests/perft.h"
#include "uci.h"


void init(Board &boardInstance);
void gameLoop(Board &boardInstance);



bool isPlaying = true;


int main(){

    const std::string CUSTOM_TEST_POSITION = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
    Game activeGame;
    init(activeGame.board);

    init(activeGame.board);
    std::cout << "Starting protocol communication" << std::endl;
    readLoop(activeGame);
    return 0;
}

void init(Board &boardInstance){

    std::cout << "Initialising engine... \n";
    
    precomputeBitBoardMoves();
}







