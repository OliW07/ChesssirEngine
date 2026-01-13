#include <gtest/gtest.h>
#include "board.h"
#include "Types.h"
#include "engine.h"

struct Puzzle {
    std::string fen;
    int depth;
    std::string bestMove;
};


class PuzzleTestFixture : public ::testing::TestWithParam<Puzzle> {
};

std::ostream& operator<<(std::ostream& os, const Puzzle& test) {

    return os << "{ FEN: " << test.fen << ", Best move: " << test.bestMove << " }";
}

TEST_P(PuzzleTestFixture,CheckMoveGen){
    Puzzle testCase = GetParam();

    Game game;
    game.setPosition(testCase.fen,MoveList{});
    game.info.depth = testCase.depth;
    std::string engineMove = convertMoveToAlgebraicNotation(game.chesssir.search());

    EXPECT_EQ(testCase.bestMove,engineMove);
    

}

INSTANTIATE_TEST_SUITE_P(
    Positions,
    PuzzleTestFixture,
    ::testing::Values(
        Puzzle{"k7/4q1n1/8/5P2/6N1/8/6PB/6NK w - - 0 1",7,"f5f6"},
        Puzzle{"1b5k/pp2n3/8/3P1Q2/8/8/8/7K b - - 0 1",3,"e7f5"},
        Puzzle{"1b5k/pp2n3/8/3P1Q2/8/8/8/7K b - - 0 1",1,"e7f5"},
        Puzzle{"1b5k/pp2n3/8/3P1Q2/8/8/8/7K b - - 0 1",2,"e7f5"},
        Puzzle{"6k1/5ppp/8/8/8/8/1R6/6K1 w - - 0 1",2,"b2b8"},
        Puzzle{"r3r1k1/2p1b1p1/p1p2p2/2P5/PP1p2N1/3q1Pp1/6K1/2R1Q2R w - - 0 35", 4,"e1e6"}
    )

);
