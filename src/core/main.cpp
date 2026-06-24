#include <cstdio>

#include "board.h"
#include "uci.h"

int main() {
    // flush buffer for the gui immediately
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::setvbuf(stdin, NULL, _IONBF, 0);

    Game game;
    readLoop(game);

    return 0;
}
