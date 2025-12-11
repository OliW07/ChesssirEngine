#include <cstdint>
#include <map>

#include "board.h"
#include "moveGenerator.h"
#include "perft.h"


std::map<std::string,int> moveBreakDown = {};

int maximum = 4;


uint64_t perftSearch(Board &boardInstance, int maxDepth){

    if(maxDepth == 0) return 1ULL;

    MoveGenerator moveGenerator(boardInstance);
    MoveList moves = moveGenerator.getAllMoves();

    uint64_t nodeCount = 0;

    for(const auto &move : moves){

        boardInstance.makeMove(move);

        uint64_t nodes = perftSearch(boardInstance, maxDepth-1);
        nodeCount += nodes;

        boardInstance.unmakeMove(move);

        if(maxDepth == maximum){
            std::string algebraicNotation = convertMoveToAlgebraicNotation(move);
            moveBreakDown[algebraicNotation] += nodes;
        }

    }

    return nodeCount;

}
