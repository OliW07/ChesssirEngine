#include <gtest/gtest.h>
#include <iostream>
#include "perft.h"
#include "Types.h"
#include "board.h"

struct PerftCase{
    std::string fen;
    int depth;
    uint64_t expectedNodes;
};


//Readable error logging;
std::ostream& operator<<(std::ostream& os, const PerftCase& test) {

    return os << "{ FEN: " << test.fen << ", Depth: " << test.depth << " }";
}
class PerftTestFixture : public ::testing::TestWithParam<PerftCase> {
};

TEST_P(PerftTestFixture,CheckNodeCount){
    PerftCase testCase = GetParam();

    Game game;
    game.setPosition(testCase.fen,MoveList{});

    uint64_t searchedNodes = perftSearch(game,testCase.depth);

    EXPECT_EQ(searchedNodes,testCase.expectedNodes) << "Failed at fen: " << testCase.fen << " at depth " << testCase.depth;

}

INSTANTIATE_TEST_SUITE_P(
    Positions,
    PerftTestFixture,
    ::testing::Values(
        PerftCase{STARTING_FEN,1,20},
        PerftCase{STARTING_FEN,2,400},
        //Tricky position
        PerftCase{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",3,97862}
    )
);
