#include <gtest/gtest.h>

#include "Types.h"
#include "board.h"
#include "evaluate.h"

struct Position {
    std::string fen;
    std::string words;
    int eval;
};

class EvalTestFixture : public::testing::TestWithParam<Position>{};


std::ostream& operator<<(std::ostream& os, const Position& test) {

    return os << "{ FEN: " << test.fen << " }";
}

TEST_P(EvalTestFixture,CheckDraws){
    Position testCase = GetParam();

    Game game;

    std::stringstream words(testCase.words);
    std::string word;

    
    MoveList moves;
    std::vector<Move> moveHistory;

    game.setPosition(testCase.fen,{});

    int initialEval = game.board.eval;

    while(words >> word){
        Move move = convertAlgebraicNotationToMove(word);
        moves.add(move);
        moveHistory.push_back(move);
    }

    

    for(auto &move : moves){
        game.board.makeMove(move);
    }


    EXPECT_EQ(testCase.eval, game.board.eval) << "Failed at fen: " << testCase.fen << "Test Case eval: " << game.board.eval << "excepted: " << testCase.eval << std::endl;

    
    

    // Unmake the moves using the history stack
    for(int i = moveHistory.size() - 1; i >= 0; --i){
        game.board.unmakeMove(moveHistory[i]);
    }
    
    EXPECT_EQ(initialEval,game.board.eval) << "Failed at fen: " << testCase.fen << "Initial eval " << initialEval << " eval after unmaking: " << game.board.eval << std::endl;

}

INSTANTIATE_TEST_SUITE_P(
    Positions,
    EvalTestFixture,
    ::testing::Values(
        Position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "e2a6 c7c5", PieceValues[Bishop]},
        Position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "e2a6 c7c5 d5c6", -430},
        Position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "e2a6 c7c5 d5c6 a8b8 c6c7 f6e4 c7b8q", -1630},
        Position{STARTING_FEN,"",0},
        Position{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -","", 0}

    )
);
