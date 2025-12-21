#include <iostream>
#include <cctype>
#include <cmath>


#include "board.h"
#include "data/precompute.h"
#include "tests/perft.h"
#include "uci.h"
#include "engine.h"




bool isPlaying = true;


int main(){

    const std::string CUSTOM_TEST_POSITION = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";


    std::cout << "Starting protocol communication" << std::endl;
    readLoop();
    return 0;
}

void Board::init(){

    std::cout << "Initialising engine... \n";
    
    precomputeBitBoardMoves();
}







