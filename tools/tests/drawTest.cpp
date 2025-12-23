#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <iostream>

#include "board.h"
#include "Types.h"

struct DrawPosition {
    std::string fen;
    std::string words;
    bool isDraw;

};

class DrawTestFixture : public::testing::TestWithParam<DrawPosition>{};

std::ostream& operator<<(std::ostream& os, const DrawPosition& test) {

    return os << "{ FEN: " << test.fen << " }";
}
TEST_P(DrawTestFixture,CheckDraws){
    DrawPosition testCase = GetParam();

    Game game;

    std::stringstream words(testCase.words);
    std::string word;

    MoveList moves;

    while(words >> word){
        Move move = convertAlgebraicNotationToMove(word);
        moves.add(move);
    }

    game.setPosition(testCase.fen,moves);


    EXPECT_EQ(testCase.isDraw,game.isDraw()) << "Failed at fen: " << testCase.fen << std::endl;

}

INSTANTIATE_TEST_SUITE_P(
    Positions,
    DrawTestFixture,
    ::testing::Values(
        //Insufficient material
        DrawPosition{"7k/8/8/8/8/6K1/8/8 w - - 0 1","",true},
        DrawPosition{"7k/8/3N4/8/8/6K1/8/8 w - - 0 1","",true},
        DrawPosition{"7k/4B3/8/8/3b4/6K1/8/8 w - - 0 1","",true},
        DrawPosition{"7k/4B3/8/8/4b3/6K1/8/8 w - - 0 1","",false},
        DrawPosition{"7k/3nn3/8/8/8/6K1/8/8 w - - 0 1","", true},
        //2 fold repition
        DrawPosition{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1","f3g4 e8d8 g4f3 d8e8",false},
        DrawPosition{"1r2k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N3Qp/PPPBBPPP/R3K2R w KQk - 2 2","g3f3 a6e2 f3g3 e2a6",false},
        DrawPosition{STARTING_FEN,"g1f3 b8c6 f3g1 c6b8",true},
        //50 move rule
        DrawPosition{"1r2k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N3Qp/PPPBBPPP/R3K2R w KQk - 49 2","g3f3 b8a8",true}
    )
);
