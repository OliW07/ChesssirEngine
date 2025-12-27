#include <gtest/gtest.h>
#include <tranpositionTable.h>

class TTTest : public ::testing::Test{
protected:
    TranspositionTable tt{128};
};

TEST_F(TTTest, WriteAndProbe) {
    uint64_t hash = 0x123456789ABCDEF0;
    Move bestMove = {false,60,50,Queen};
    int16_t eval = 150;
    uint8_t depth = 10;
    uint8_t age = 1;
    NodeType type = NodeType::Exact;

    tt.write(hash, depth, age, eval, bestMove,type);

    TTEntry result;
    ASSERT_TRUE(tt.probe(hash, result));
    
    EXPECT_EQ(result.zhash, hash);
    EXPECT_EQ(result.eval, eval);
    EXPECT_EQ(result.depth, depth);
    EXPECT_EQ(unpackMove(result.bestMove).from, bestMove.from);
}

TEST_F(TTTest, ReplacementLogic) {
    uint64_t hash = 0xAAAA; // Same hash for both
    Move move = {false,58,57,None};

    // Write a deep entry
    tt.write(hash, 20, 1, 100, move, NodeType::Exact);
    
    // Attempt to overwrite with a shallower entry
    tt.write(hash, 5, 1, 50, move, NodeType::Exact);

    TTEntry result;
    tt.probe(hash, result);
    EXPECT_EQ(result.depth, 20); 
    EXPECT_EQ(result.eval, 100);
}
