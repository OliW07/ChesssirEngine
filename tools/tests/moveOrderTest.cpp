#include <gtest/gtest.h>

#include <moveGenerator.h>
#include <board.h>

TEST(MoveOrderTest,checkBestMove){

    MoveList moves = {};
    moves.add({false,5,0,None,50});
    moves.add({true,5,0,None,0});
    moves.add({false,8,34,None,30});
    moves.add({false,5,0,None,0});
    moves.add({false,55,40,Queen,100000});

    int answers[5] = {100000,50,30,0,0};

    for(int i = 0; i < moves.count; i++){
        moves.sortNext(i);
        
        EXPECT_EQ(moves.moves[i].orderScore, answers[i]);

    }
}
