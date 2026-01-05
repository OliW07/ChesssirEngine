#include "evaluate.h"
#include "board.h"
#include "utils/Types.h"

int generateFullEval(Board &board){
    
    int eval = 0;
    for(int i = 0; i < board.state.pieceList.pieceCount[White]; i++){
        
        Pieces pieceType = (Pieces)board.getPieceEnum(board.state.pieceList.list[White][i]);

        eval += PieceValues[static_cast<size_t>(pieceType)];
    }

    for(int i = 0; i < board.state.pieceList.pieceCount[Black]; i++){
        
        Pieces pieceType = (Pieces)board.getPieceEnum(board.state.pieceList.list[Black][i]);

        eval -= PieceValues[static_cast<size_t>(pieceType)];
    }

    if(!board.state.whiteToMove) eval *= -1;
   
    return eval;


}


void setFullEval(Board &board){

    board.eval = generateFullEval(board);
    
}

