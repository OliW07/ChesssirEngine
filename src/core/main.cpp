#include <iostream>
#include <cctype>
#include <cmath>
#include <cstdio>

#include "uci.h"
#include "board.h"

int main(){
    //flush buffer for the gui immediately
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::setvbuf(stdin, NULL, _IONBF, 0);

    Game game;
    readLoop(game);

    return 0;
}








