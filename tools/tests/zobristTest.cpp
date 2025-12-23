#include "Types.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <sstream>

#include <zobrist.h>

struct GamePlay {
    std::string fen;
    std::string moves;
};

class GamePlayTestFixture : public ::testing::TestWithParam<GamePlay>{};

TEST_P(GamePlayTestFixture,CheckZobristHash){
    GamePlay testCase = GetParam();

    Game game;
    game.setPosition(testCase.fen,MoveList{});

    uint64_t startingHash = game.board.state.zhash;

    MoveList moves;
    std::stringstream words(testCase.moves);
    std::string word;

    while(words >> word){
        Move move = convertAlgebraicNotationToMove(word);
        moves.add(move);
    }

    for(auto &move : moves){
        game.board.makeMove(move);
    }

    //Unmake the moves

    for(int i = moves.count-1; i >=0; --i){
        game.board.unmakeMove(moves.moves[i]);
    }

    EXPECT_EQ(startingHash,game.board.state.zhash);
}

INSTANTIATE_TEST_CASE_P(
    Positions,
    GamePlayTestFixture,
    ::testing::Values(
        GamePlay{STARTING_FEN,"e2e4 d7d5 e4d5 d8d5 g1f3 d5f3 f1b5 c8d7 e1g1 f3g2"},
        // En Passant Capture
        GamePlay{STARTING_FEN, "e2e4 g8f6 e4e5 d7d5 e5d6"},
        // Castling Rights lost by moving King/Rook
        GamePlay{"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1e2 a8c8 e2e1 c8a8"},
        // Promotion to Queen
        GamePlay{"8/4P3/8/8/8/8/4k3/4K3 w - - 0 1", "e7e8q e2d3 e8a4"},
        // Rook capture affecting castling rights
        GamePlay{"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "a1a8 h8h1 e1f2"}
    )
);
