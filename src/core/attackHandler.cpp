#include <stdexcept>

#include "moveGenerator.h"
#include "utils/Types.h"
#include "attackHandler.h"
#include "precompute.h"
#include "board.h"
#include "utils/bitops.h"

using namespace precomputedData;
using namespace ChessEngine::Utils;


uint64_t AttackHandler::getAttacks(const int pos){

    uint64_t PieceAttacks = 0ULL;
    uint64_t target = 1ULL << pos;

    
    if(game.board.isSquareEmpty(pos)) return 0ULL;

    enum Pieces pieceType = (Pieces)game.board.getPieceEnum(pos);

    int start = 0, end = 0;

    switch(pieceType){
        case Pawn:
            PieceAttacks = game.board.isPieceWhite(pos) ? whitePawnAttacks[pos] : blackPawnAttacks[pos];
            //The pawn can only attack (diagonally) is there is an enemy piece there
            PieceAttacks &= (game.board.isPieceWhite(pos) ? game.board.state.occupancy[Black] : game.board.state.occupancy[White]);
            break;
        case Knight:
            PieceAttacks = knightMoves[pos];
            break;
        case Bishop:
            PieceAttacks = bishopMoves[pos];
            start = 4;
            end = 8;
            break;
        case Rook:
            PieceAttacks = rookMoves[pos];
            start = 0;
            end = 4;
            break;
        case King:
            PieceAttacks = kingMoves[pos];
            break;
        case Queen:
            PieceAttacks = queenMoves[pos];
            start = 0;
            end = 8;
            break;
        default:
            throw std::runtime_error("Invalid piecetype: ");
            break;
        
    }

    //Find which piece it at the position

    uint64_t blockingRays = 0ULL;
    uint64_t blockers = PieceAttacks & game.board.state.occupancy[Both];

    const RaysDirection directions[8] = {North,East,South,West,NorthEast,SouthEast,SouthWest,NorthWest};


    //calculate blockingRays in every direction for the different sliding pieces

    for(int i = start; i < end; i++){

        if(!blockers) break;

        uint64_t blockers1Direction = blockers & rays[directions[i]][pos];

        if(!blockers1Direction) continue;

        int firstBlockerPos = game.board.getFirstBlocker(pos,directions[i]);
	
	if(firstBlockerPos == -1) continue;
        
        blockingRays |= rays[directions[i]][firstBlockerPos];    
    
    }


    uint64_t attacks = (PieceAttacks) & (~game.board.getFriendlyPieces(pos)) & (~blockingRays);

    if(pieceType == Pawn && game.board.state.enPassantSquare != -1){
        
        bool pieceWraps = pieceWrapsTheBoard(game.board.state.enPassantSquare,pos);

        //The pawn must be diagonally next to the enpassant square to move there
        if((game.board.isPieceWhite(pos) && ((game.board.state.enPassantSquare - 9 == pos) || (game.board.state.enPassantSquare - 7 == pos))) && !pieceWraps){

            attacks |= (1ULL << game.board.state.enPassantSquare);
        }else if(!game.board.isPieceWhite(pos) && ((game.board.state.enPassantSquare + 9 == pos) || (game.board.state.enPassantSquare + 7 == pos))&& !pieceWraps){

            attacks |= (1ULL << game.board.state.enPassantSquare);
        }
 
    }

    return attacks;

}

uint64_t AttackHandler::getAllAttacks(bool isWhite){

    uint64_t attacks = 0ULL;
    uint64_t pieceBitBoard = isWhite ? game.board.state.occupancy[White] : game.board.state.occupancy[Black];

    
    while(pieceBitBoard){

	int location = ctz64(pieceBitBoard);
	pieceBitBoard &= (pieceBitBoard-1);

        attacks |= getAttacks(location);
    }

    return attacks;
}

uint64_t AttackHandler::getAttackers(int pos, bool attackingIsWhite){

    uint64_t attackers = 0ULL;

    attackers |= getPawnAttackers(pos, attackingIsWhite);

    attackers |= (attackingIsWhite ? (knightMoves[pos] & game.board.state.bitboards[White][Knight]) : (knightMoves[pos] & game.board.state.bitboards[Black][Knight]));

    for (auto &[direction,offset] : Compass){

        int attackingPiecePos = game.board.getFirstBlocker(pos, direction);
	
	if(attackingPiecePos == -1) continue;

        Pieces pieceType = (Pieces)game.board.getPieceEnum(attackingPiecePos);

        if(game.board.isPieceWhite(attackingPiecePos) != attackingIsWhite) continue;

        bool straightSliding = (direction == North || direction == East || direction == West || direction == South);

        if((straightSliding && ((pieceType == Rook) || pieceType == Queen)) || 
          (!straightSliding && ((pieceType == Bishop || pieceType == Queen)))){

            attackers |= (1ULL << attackingPiecePos);
        }
    }

    return attackers;

}

uint64_t AttackHandler::getPawnAttackers(int pos, bool attackingIsWhite){

    if(!posInBounds(pos)) return 0ULL;

    uint64_t attackers = 0ULL;
    uint64_t *pieceBitBoard = &game.board.state.bitboards[attackingIsWhite][Pawn];

    int offsets[2] = {7,9};

    for(int direction : offsets){

        int pawnPos = attackingIsWhite ? pos-direction : pos+direction;

        if(!posInBounds(pawnPos)) continue;

        bool pieceWraps = pieceWrapsTheBoard(pos, pawnPos);

        if((*pieceBitBoard & (1ULL << pawnPos)) && !pieceWraps){

            attackers |= (1ULL << pawnPos);

        }
    }

    return attackers;

}

uint64_t AttackHandler::pawnControlledSquare(bool controllingColourIsWhite){

    uint64_t pawnBitBoard = controllingColourIsWhite ? game.board.state.bitboards[White][Pawn] : game.board.state.bitboards[Black][Pawn];
    uint64_t *pawnAttacks = controllingColourIsWhite ? whitePawnAttacks : blackPawnAttacks;
    uint64_t controlledSquares = 0ULL;
    
    while(pawnBitBoard){
        int pawnPos = ctz64(pawnBitBoard);
        controlledSquares |= pawnAttacks[pawnPos];
        pawnBitBoard ^= (1ULL << pawnPos);
    }

    return controlledSquares;

}


uint64_t AttackHandler::getPinnedPieces(bool isWhite, bool includeEnemies){
    uint64_t pinnedPieces = 0ULL;
    uint64_t kingLocation = game.board.getKingLocation(isWhite);
    
    for (auto const &[direction,offset] : Compass){

        uint64_t ray = rays[direction][kingLocation];

        int firstBlocker = game.board.getFirstBlocker(kingLocation, direction);
        if(firstBlocker == -1) continue;

        //Pinned piece must be a friendly piece
        if(game.board.isPieceWhite(firstBlocker) != game.board.isPieceWhite(kingLocation) && !includeEnemies )  continue;

        int secondBlocker = game.board.getFirstBlocker(firstBlocker,direction);
        if(secondBlocker == -1) continue;

        //If the second piece is also a friendly, nothing is pinned
        if(game.board.isPieceWhite(secondBlocker) == game.board.isPieceWhite(kingLocation)) continue;

        enum Pieces enemyPiece = (Pieces)game.board.getPieceEnum(secondBlocker);

        if((enemyPiece == Bishop || enemyPiece == Queen) && (direction == NorthEast || direction == NorthWest || direction == SouthEast || direction == SouthWest)){

            pinnedPieces |= (1ULL << firstBlocker);

        }else if((enemyPiece == Rook || enemyPiece == Queen) && (direction == North || direction == South || direction == East || direction == West)){

            pinnedPieces |= (1ULL << firstBlocker);

        }
         
    }

    return pinnedPieces;

}


bool AttackHandler::isSquareAttacked(int pos, bool attackingColourIsWhite){
    //return (1ULL << pos & getAllAttacks(attackingColourIsWhite));
    if (getPawnAttackers(pos, attackingColourIsWhite)) return true;

    uint64_t enemyKnights = attackingColourIsWhite ? game.board.state.bitboards[White][Knight] : game.board.state.bitboards[Black][Knight];
    uint64_t enemyKing    = attackingColourIsWhite ? game.board.state.bitboards[White][King]   : game.board.state.bitboards[Black][King];
    uint64_t enemyRooks   = attackingColourIsWhite ? game.board.state.bitboards[White][Rook]   : game.board.state.bitboards[Black][Rook];
    uint64_t enemyBishops = attackingColourIsWhite ? game.board.state.bitboards[White][Bishop] : game.board.state.bitboards[Black][Bishop];
    uint64_t enemyQueens  = attackingColourIsWhite ? game.board.state.bitboards[White][Queen]  : game.board.state.bitboards[Black][Queen];

    if(knightMoves[pos] & enemyKnights) return true;
    if(kingMoves[pos] & enemyKing) return true;
    
    RaysDirection diagonals[4] = {NorthEast,NorthWest,SouthEast,SouthWest};
    RaysDirection straights[4] = {North,South,East,West};

    for(RaysDirection direction : diagonals){

        int attackerPos = game.board.getFirstBlocker(pos, direction);
        if(attackerPos == -1) continue;

        if((1ULL << attackerPos) & (enemyBishops | enemyQueens)) return true;

    }

    for(RaysDirection direction : straights){

         int attackerPos = game.board.getFirstBlocker(pos, direction);
         if(attackerPos == -1) continue;

         if((1ULL << attackerPos) & (enemyRooks | enemyQueens)) return true;
    }

    return false;

}

