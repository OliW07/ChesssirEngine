#include <stdexcept>

#include "moveGenerator.h"
#include "utils/Types.h"
#include "attackHandler.h"
#include "precompute.h"
using namespace precomputedData;


uint64_t AttackHandler::getAttacks(const int pos){

    uint64_t PieceAttacks = 0ULL;
    uint64_t target = 1ULL << pos;

    uint64_t allPieces = board.state.whitePieceBitBoard | board.state.blackPieceBitBoard;

    
    if(board.isSquareEmpty(pos)) return 0ULL;

    enum Pieces pieceType = (Pieces)board.getPieceEnum(pos);

    int start = 0, end = 0;

    switch(pieceType){
        case Pawn:
            PieceAttacks = board.isPieceWhite(pos) ? whitePawnAttacks[pos] : blackPawnAttacks[pos];
            //The pawn can only attack (diagonally) is there is an enemy piece there
            PieceAttacks &= (board.isPieceWhite(pos) ? board.state.blackPieceBitBoard : board.state.whitePieceBitBoard);
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
    uint64_t blockers = PieceAttacks & allPieces;

    const RaysDirection directions[8] = {North,East,South,West,NorthEast,SouthEast,SouthWest,NorthWest};


    //calculate blockingRays in every direction for the different sliding pieces

    for(int i = start; i < end; i++){

        if(!blockers) break;

        uint64_t blockers1Direction = blockers & rays[directions[i]][pos];

        if(!blockers1Direction) continue;

        int firstBlockerPos = board.getFirstBlocker(pos,directions[i]);
	
	if(firstBlockerPos == -1) continue;
        
        blockingRays |= rays[directions[i]][firstBlockerPos];    
    
    }


    uint64_t attacks = (PieceAttacks) & (~board.getFriendlyPieces(pos)) & (~blockingRays);

    if(pieceType == Pawn && board.state.enPassantSquare != -1){
        
        bool pieceWraps = pieceWrapsTheBoard(board.state.enPassantSquare,pos);

        //The pawn must be diagonally next to the enpassant square to move there
        if((board.isPieceWhite(pos) && ((board.state.enPassantSquare - 9 == pos) || (board.state.enPassantSquare - 7 == pos))) && !pieceWraps){

            attacks |= (1ULL << board.state.enPassantSquare);
        }else if(!board.isPieceWhite(pos) && ((board.state.enPassantSquare + 9 == pos) || (board.state.enPassantSquare + 7 == pos))&& !pieceWraps){

            attacks |= (1ULL << board.state.enPassantSquare);
        }
 
    }

    return attacks;

}

uint64_t AttackHandler::getAllAttacks(bool isWhite){

    uint64_t attacks = 0ULL;
    uint64_t pieceBitBoard = isWhite ? board.state.whitePieceBitBoard : board.state.blackPieceBitBoard;

    
    while(pieceBitBoard){

	int location = __builtin_ctzll(pieceBitBoard);
	pieceBitBoard &= (pieceBitBoard-1);

        attacks |= getAttacks(location);
    }

    return attacks;
}

uint64_t AttackHandler::getAttackers(int pos, bool attackingIsWhite){

    uint64_t attackers = 0ULL;

    attackers |= getPawnAttackers(pos, attackingIsWhite);

    attackers |= (attackingIsWhite ? (knightMoves[pos] & board.state.whiteKnightBitBoard) : (knightMoves[pos] & board.state.blackKnightBitBoard));

    for (auto &[direction,offset] : Compass){

        int attackingPiecePos = board.getFirstBlocker(pos, direction);
	
	if(attackingPiecePos == -1) continue;

        Pieces pieceType = (Pieces)board.getPieceEnum(attackingPiecePos);

        if(board.isPieceWhite(attackingPiecePos) != attackingIsWhite) continue;

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
    uint64_t *pieceBitBoard = attackingIsWhite ? &board.state.whitePawnBitBoard : &board.state.blackPawnBitBoard;

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

    uint64_t pawnBitBoard = controllingColourIsWhite ? board.state.whitePawnBitBoard : board.state.blackPawnBitBoard;
    uint64_t *pawnAttacks = controllingColourIsWhite ? whitePawnAttacks : blackPawnAttacks;
    uint64_t controlledSquares = 0ULL;
    
    while(pawnBitBoard){
        int pawnPos = __builtin_ctzll(pawnBitBoard);
        controlledSquares |= pawnAttacks[pawnPos];
        pawnBitBoard ^= (1ULL << pawnPos);
    }

    return controlledSquares;

}


uint64_t AttackHandler::getPinnedPieces(bool isWhite, bool includeEnemies){
    uint64_t pinnedPieces = 0ULL;
    uint64_t kingLocation = board.getKingLocation(isWhite);
    
    for (auto const &[direction,offset] : Compass){

        uint64_t ray = rays[direction][kingLocation];

        int firstBlocker = board.getFirstBlocker(kingLocation, direction);
        if(firstBlocker == -1) continue;

        //Pinned piece must be a friendly piece
        if(board.isPieceWhite(firstBlocker) != board.isPieceWhite(kingLocation) && !includeEnemies )  continue;

        int secondBlocker = board.getFirstBlocker(firstBlocker,direction);
        if(secondBlocker == -1) continue;

        //If the second piece is also a friendly, nothing is pinned
        if(board.isPieceWhite(secondBlocker) == board.isPieceWhite(kingLocation)) continue;

        enum Pieces enemyPiece = (Pieces)board.getPieceEnum(secondBlocker);

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

    uint64_t enemyKnights = attackingColourIsWhite ? board.state.whiteKnightBitBoard : board.state.blackKnightBitBoard;
    uint64_t enemyKing    = attackingColourIsWhite ? board.state.whiteKingBitBoard   : board.state.blackKingBitBoard;
    uint64_t enemyRooks   = attackingColourIsWhite ? board.state.whiteRookBitBoard   : board.state.blackRookBitBoard;
    uint64_t enemyBishops = attackingColourIsWhite ? board.state.whiteBishopBitBoard : board.state.blackBishopBitBoard;
    uint64_t enemyQueens  = attackingColourIsWhite ? board.state.whiteQueenBitBoard  : board.state.blackQueenBitBoard;

    if(knightMoves[pos] & enemyKnights) return true;
    if(kingMoves[pos] & enemyKing) return true;
    
    RaysDirection diagonals[4] = {NorthEast,NorthWest,SouthEast,SouthWest};
    RaysDirection straights[4] = {North,South,East,West};

    for(RaysDirection direction : diagonals){

        int attackerPos = board.getFirstBlocker(pos, direction);
        if(attackerPos == -1) continue;

        if((1ULL << attackerPos) & (enemyBishops | enemyQueens)) return true;

    }

    for(RaysDirection direction : straights){

         int attackerPos = board.getFirstBlocker(pos, direction);
         if(attackerPos == -1) continue;

         if((1ULL << attackerPos) & (enemyRooks | enemyQueens)) return true;
    }

    return false;

}

