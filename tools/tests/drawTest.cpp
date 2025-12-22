#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <iostream>

#include "board.h"
#include "Types.h"

struct DrawPosition {
    std::string fen;
    bool isDraw;

};

class DrawTestFixture : public::testing::TestWithParam<DrawPosition>{};

TEST_P(DrawTestFixture,CheckDraws){
    DrawPosition testCase = GetParam();

    Game game;
    game.setPosition(testCase.fen,MoveList{});

    EXPECT_EQ(testCase.isDraw,game.isDraw()) << "Failed at fen: " << testCase.fen << std::endl;

}

INSTANTIATE_TEST_SUITE_P(
    Positions,
    DrawTestFixture,
    ::testing::Values(
        DrawPosition{"7k/8/8/8/8/6K1/8/8 w - - 0 1",true},
        DrawPosition{"7k/8/3N4/8/8/6K1/8/8 w - - 0 1",true},
        DrawPosition{"7k/4B3/8/8/3b4/6K1/8/8 w - - 0 1",true},
        DrawPosition{"7k/4B3/8/8/4b3/6K1/8/8 w - - 0 1",false},
        DrawPosition{"7k/3nn3/8/8/8/6K1/8/8 w - - 0 1", true},
        DrawPosition{"7k/3n4/8/8/4N3/6K1/8/8 w - - 0 1",true},
        DrawPosition{"7k/3n4/8/8/4N3/P5K1/8/8 w - - 0 1",false},
        DrawPosition{"4BB1k/8/8/8/8/6K1/8/8 w - - 0 1",false}
    )
);
