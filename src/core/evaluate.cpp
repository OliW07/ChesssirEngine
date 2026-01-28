#include "evaluate.h"
#include "board.h"
#include "utils/Types.h"
#include "precompute.h"

int generateFullEval(Board &board){
    
    int eval = 0;
    for(int i = 0; i < board.state.pieceList.pieceCount[White]; i++){
        
        int pos = board.state.pieceList.list[White][i];
        Pieces pieceType = (Pieces)board.getPieceEnum(pos);

        eval += PieceValues[static_cast<size_t>(pieceType)];
        eval += evaluatePieceSquare(pieceType, pos, true, false);
    }

    for(int i = 0; i < board.state.pieceList.pieceCount[Black]; i++){
        
        int pos = board.state.pieceList.list[Black][i];
        Pieces pieceType = (Pieces)board.getPieceEnum(pos);

        eval -= PieceValues[static_cast<size_t>(pieceType)];
        eval += evaluatePieceSquare(pieceType, pos, false, false);
    }

    return eval;


}


void setFullEval(Board &board){

    board.eval = generateFullEval(board);
    
}

