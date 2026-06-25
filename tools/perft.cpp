#include "perft.h"

#include <cstdint>
#include <map>

#include "board.h"
#include "moveGenerator.h"

uint64_t perftSearch(Game& game, int maxDepth, int initialDepth, std::map<std::string, int>* outBreakdown) {
    if (maxDepth == 0)
        return 1ULL;

    MoveList moves = game.moveGenerator.getAllMoves();

    uint64_t nodeCount = 0;

    for (const auto& move : moves) {
        game.board.makeMove(move);

        uint64_t nodes = perftSearch(game, maxDepth - 1, initialDepth, outBreakdown);
        nodeCount += nodes;

        game.board.unmakeMove(move);

        if (maxDepth == initialDepth && outBreakdown) {
            std::string algebraicNotation = convertMoveToAlgebraicNotation(move);
            (*outBreakdown)[algebraicNotation] += nodes;
        }
    }

    return nodeCount;
}
