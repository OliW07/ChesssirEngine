#include "evaluate.h"
#include "utils/Types.h"



int evaluateState(Board &boardInstance){
    

    int eval = 0;
    
    for(int i = 0; i < boardInstance.state.pieceList.pieceCount[White]; i++){
        
        Pieces pieceType = (Pieces)boardInstance.getPieceEnum(boardInstance.state.pieceList.list[White][i]);

        eval += PieceValues[static_cast<size_t>(pieceType)];
    }

    for(int i = 0; i < boardInstance.state.pieceList.pieceCount[Black]; i++){
        
        Pieces pieceType = (Pieces)boardInstance.getPieceEnum(boardInstance.state.pieceList.list[Black][i]);

        eval -= PieceValues[static_cast<size_t>(pieceType)];
    }

    return eval;
}

