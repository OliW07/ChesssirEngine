#include <cstdint>
#include <map>

#include "board.h"
#include "moveGenerator.h"
#include "perft.h"


std::map<std::string,int> moveBreakDown = {};

int maximum = 2;



uint64_t perftSearch(Board &boardInstance, int maxDepth){

    MoveGenerator moveGenerator(boardInstance);

    if(maxDepth == 0) return 1;

    uint64_t nodeCount = 0;
        
    for(int i = 0; i < 12; i++){

        

        bool isWhite = i < 6;
        if(isWhite != boardInstance.state.whiteToMove) continue;

        uint64_t pieceLocations = *boardInstance.state.p_pieceBitBoards[i];

        

	while(pieceLocations){
        
    	    int pieceLoc = __builtin_ctzll(pieceLocations);
	    //Effeciently toggle off the least significant bit
	    pieceLocations &= (pieceLocations- 1);			


	    uint64_t normalMoves = moveGenerator.getLegalMoves(pieceLoc);
	    uint64_t promotionMoves = moveGenerator.getPromotionMoves(pieceLoc);
	
	

	    while(normalMoves){
                
		    uint64_t move = __builtin_ctzll(normalMoves);
		    normalMoves &= (normalMoves - 1);

            BoardState savedState = boardInstance.state;
            boardInstance.makeMove(pieceLoc,move,-1);

            uint64_t nodes = perftSearch(boardInstance,maxDepth-1);
            nodeCount+=nodes;

            boardInstance.state = savedState;

            if(maxDepth == maximum){
                std::string algebraicNotation = convertMoveToAlgebraicNotation(pieceLoc)+convertMoveToAlgebraicNotation(move);
                if(!moveBreakDown[algebraicNotation]) moveBreakDown[algebraicNotation] = nodes;
                else moveBreakDown[algebraicNotation]+=nodes;
            }

        }

	    while (promotionMoves){


		uint64_t move = __builtin_ctzll(promotionMoves);
		promotionMoves &= (promotionMoves - 1);
		
                uint64_t subNodes = 0;

                for(int promotionPiece : {Rook,Bishop,Knight,Queen}){

                    
                    
                    //For each promotion piece enum Rook, Bishop, Knight & Queen
                    BoardState savedState = boardInstance.state;
                    boardInstance.makeMove(pieceLoc,move,promotionPiece);

                    uint64_t nodes = perftSearch(boardInstance,maxDepth-1);
                    subNodes+=nodes;

                    boardInstance.state = savedState;

                }

                if(maxDepth == maximum){
                    std::string algebraicNotation = convertMoveToAlgebraicNotation(pieceLoc)+convertMoveToAlgebraicNotation(move);
                    if(!moveBreakDown[algebraicNotation]) moveBreakDown[algebraicNotation] = subNodes;
                    else moveBreakDown[algebraicNotation]+=subNodes;
                }
                nodeCount+=subNodes;
            }

           


        }

        
        

    }

    return nodeCount;

}
