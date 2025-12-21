#include <cstdint>
#include <map>

#include "board.h"
#include "moveGenerator.h"
#include "perft.h"


std::map<std::string,int> moveBreakDown = {};

int maximum = 4;


uint64_t perftSearch(Game &game, int maxDepth){

    if(maxDepth == 0) return 1ULL;

    MoveList moves = game.moveGenerator.getAllMoves();

    uint64_t nodeCount = 0;

    for(const auto &move : moves){

        game.board.makeMove(move);

        uint64_t nodes = perftSearch(game, maxDepth-1);
        nodeCount += nodes;

        game.board.unmakeMove(move);

        if(maxDepth == maximum){
            std::string algebraicNotation = convertMoveToAlgebraicNotation(move);
            moveBreakDown[algebraicNotation] += nodes;
        }

    }

    return nodeCount;

}
