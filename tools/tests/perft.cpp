#include <cstdint>
#include <map>

#include "board.h"
#include "moveGenerator.h"
#include "perft.h"


std::map<std::string,int> moveBreakDown = {};

int maximum = 5;



uint64_t perftSearch(Board &boardInstance, int maxDepth){

    MoveGenerator moveGenerator(boardInstance);

    if(maxDepth == 0) return 1;

    uint64_t nodeCount = 0;
    
    for(int j = Black; j <= White; j++){


        bool isWhite = (j == White);
        if(isWhite != boardInstance.state.whiteToMove) continue;


        for(int i = 0; i < 6; i++){

            uint64_t pieceLocations = boardInstance.state.bitboards[j][i];

            while(pieceLocations){
                
                int pieceLoc = __builtin_ctzll(pieceLocations);
                //Effeciently toggle off the least significant bit
                pieceLocations &= (pieceLocations- 1);			


                uint64_t normalMoves = moveGenerator.getLegalMoves(pieceLoc);
                uint64_t promotionMoves = moveGenerator.getPromotionMoves(pieceLoc);
            
                Move move;
                move.from = pieceLoc;

                while(normalMoves){
                       


                    move.to = __builtin_ctzll(normalMoves);
                    normalMoves &= (normalMoves - 1);
            
                    boardInstance.makeMove(move);

                    uint64_t nodes = perftSearch(boardInstance,maxDepth-1);
                    nodeCount+=nodes;

                    boardInstance.unmakeMove(move);

                    if(maxDepth == maximum){
                        std::string algebraicNotation = convertMoveToAlgebraicNotation(move);
                        if(!moveBreakDown[algebraicNotation]) moveBreakDown[algebraicNotation] = nodes;
                        else moveBreakDown[algebraicNotation]+=nodes;
                    }

                }

                while (promotionMoves){


                    move.to = __builtin_ctzll(promotionMoves);
                    promotionMoves &= (promotionMoves - 1);

                    
                    uint64_t subNodes = 0;

                    for(Pieces promotionPiece : {Rook,Bishop,Knight,Queen}){

                        
                        move.promotionPiece = promotionPiece;
                        
                        //For each promotion piece enum Rook, Bishop, Knight & Queen
                        boardInstance.makeMove(move);

                        uint64_t nodes = perftSearch(boardInstance,maxDepth-1);
                        subNodes+=nodes;

                        boardInstance.unmakeMove(move);

                    }

                    if(maxDepth == maximum){
                        std::string algebraicNotation = convertMoveToAlgebraicNotation(move);
                        if(!moveBreakDown[algebraicNotation]) moveBreakDown[algebraicNotation] = subNodes;
                        else moveBreakDown[algebraicNotation]+=subNodes;
                    }
                    nodeCount+=subNodes;
                }

                   


            }

            
            

        }


    }

    return nodeCount;

}
