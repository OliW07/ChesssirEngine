#include <cstdlib>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <algorithm>

#include "board.h"
#include "Types.h"
#include "precompute.h"
#include "utils/Types.h"
#include "debug.h"

extern uint64_t KnightMoves[64];
extern uint64_t KingMoves[64];
extern uint64_t QueenMoves[64];
extern uint64_t RookMoves[64];
extern uint64_t BishopMoves[64];
extern uint64_t whitePawnMoves[64];
extern uint64_t blackPawnMoves[64];
extern uint64_t whitPawnAttacks[64];
extern uint64_t blackPawnAttacks[64];
extern uint64_t Rays[8][64];

Board::Board(const std::string fen, bool isAdversaryWhite){
    this->isAdversaryWhite = isAdversaryWhite;
    parseFenString(fen);
}


uint64_t Board::getPseudoLegalMoves(const int pos){

    if(isSquareEmpty(pos)) return 0ULL;

    uint64_t allPieces = state.whitePieceBitBoard | state.blackPieceBitBoard;
    uint64_t pseudoLegalMoves = getAttacks(pos);
    
    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    //Add (non attack) pawn moves 
    if(pieceType == Pawn){

        uint64_t *pawnMoves = isPieceWhite(pos) ? &whitePawnMoves[pos] : &blackPawnMoves[pos];
    
        //Pawns can't move onto their own pieces or friendly pieces
        pseudoLegalMoves |= (*pawnMoves & ~allPieces);

        uint64_t oneSpaceTargetSquare = isPieceWhite(pos) ? (1ULL << std::min(pos+8,64)) : (1ULL << std::max(pos-8,-1));

        //If a piece in directly infront of a pawn, it can't do two moves
        if((oneSpaceTargetSquare & allPieces) && oneSpaceTargetSquare != 64 && oneSpaceTargetSquare != -1){
            if(isPieceWhite(pos)) pseudoLegalMoves &= ~(1ULL << pos+16);
            else  pseudoLegalMoves &= ~(1ULL << pos-16);
        }
    }

    //Add castling

    if(pieceType == King && state.castlingRights > 0){

        
        if(isPieceWhite(pos) && (state.castlingRights & 8)  && !isSquareAttacked(5,false) && !isSquareAttacked(6,false) && isSquareEmpty(5) && isSquareEmpty(6)){
                //White kingside castling square
                    pseudoLegalMoves |= (1ULL << 6);
        }else if(isPieceWhite(pos) && (state.castlingRights & 4) && !isSquareAttacked(3,false) && !isSquareAttacked(2,false) && !isSquareAttacked(1,false) && isSquareEmpty(3) && isSquareEmpty(2) && isSquareEmpty(1)){
                    //White queenside castling square
                    pseudoLegalMoves |= (1ULL << 2);
                }
        else if(!isPieceWhite(pos) && (state.castlingRights & 2)&& !isSquareAttacked(61,true) && !isSquareAttacked(62,true) && isSquareEmpty(61) && isSquareEmpty(62)){
            
            //Black kingside castling square
            pseudoLegalMoves |= (1ULL << 62);

        }else if(!isPieceWhite(pos) && (state.castlingRights & 1)&& !isSquareAttacked(57,true) && !isSquareAttacked(58,true) && !isSquareAttacked(59,true)&& isSquareEmpty(57) && isSquareEmpty(58) && isSquareEmpty(59)){
          //Black queenside castling square
            pseudoLegalMoves |= (1ULL << 58);
        }
}


    return pseudoLegalMoves;

}

uint64_t Board::applyLegalMoveValidation(const int pos, uint64_t moves){

    uint64_t legalMoves = moves;

    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    bool isWhite = isPieceWhite(pos);

    int kingLocation = getKingLocation(isWhite);

    
    if(pieceType == King){

        //For each move, check the square is not controlled by the enemy and disallow king from castling whilst in check
        uint64_t movesClone = legalMoves;

        while(movesClone){

            int move = __builtin_ctzll(movesClone);
            movesClone &= (movesClone - 1);

            if(isSquareAttacked(move,!isWhite)) legalMoves ^= (1ULL << move);  

            if(abs(convertLocationToColumns(move) - convertLocationToColumns(pos)) > 1 && (isSquareAttacked(pos, !isWhite))) legalMoves ^= (1ULL << move);


        }

	    //For each sliding piece attacker, remove the squares that the king hides inline with the attacking ray
	
        uint64_t attackers = getAttackers(kingLocation,!isWhite);
        

        while(attackers){


            int attackerLoc = __builtin_ctzll(attackers);
            attackers &= (attackers-1);

            enum Pieces attackerType = (Pieces)getPieceEnum(attackerLoc);

            if(attackerType == Knight || attackerType == King || attackerType == Pawn) continue;


            RaysDirection direction = convertPositionsToDirections(attackerLoc,pos);
            legalMoves &= ~Rays[direction][attackerLoc];
            
        }	
        

        
    } 

    else if(isSquareAttacked(kingLocation,!isWhite)){
        
        uint64_t attackers = getAttackers(kingLocation,!isWhite);

        //If the king is attacked more than once, the king is the only piece that can move;
        if(countOnes(attackers) > 1) return 0ULL;

        int attackerLocation = __builtin_ctzll(attackers);

	
        uint64_t blockingCapturingRay;
	
	if(getPieceEnum(attackerLocation) == Knight){
		blockingCapturingRay = attackers;
	}else{
		blockingCapturingRay = getRay(kingLocation,attackerLocation);
	}

        legalMoves &= blockingCapturingRay;

    }
    //If the piece is pinned, limit its movement to the ray between itself and the king
    if(getPinnedPieces(isWhite) & (1ULL << pos)){

        int kingLocation = getKingLocation(isWhite);

        RaysDirection direction = convertPositionsToDirections(kingLocation,pos);

        legalMoves &= Rays[direction][kingLocation];
    }

    return legalMoves;
}

uint64_t Board::getLegalMoves(const int pos){

    //Legal moves, except for pawn moves to promotion squares, handled seperately

    if(pos < 0 || pos > 63) throw std::runtime_error("The position provided, is not in range 0-63");

    uint64_t legalMoves = applyLegalMoveValidation(pos,getPseudoLegalMoves(pos));

    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    bool isWhite = isPieceWhite(pos);

    //Remove pawns promting, handling seperately with multiple outcomes

    if(pieceType == Pawn){
        int promotionRow = isWhite ? 7 : 0;
        legalMoves &= ~(RankMasks[promotionRow]);
    }

    return legalMoves;

}

uint64_t Board::getPromotionMoves(const int pos){

    Pieces pieceType = (Pieces)getPieceEnum(pos);
    if(pieceType != Pawn) return 0ULL;

    uint64_t promotionRank = isPieceWhite(pos) ? 7 : 0;

    uint64_t promotionMoves = getPseudoLegalMoves(pos) & RankMasks[promotionRank];

    return applyLegalMoveValidation(pos, promotionMoves);

}

uint64_t Board::getAttacks(const int pos){

    uint64_t PieceAttacks = 0ULL;
    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.whitePieceBitBoard | state.blackPieceBitBoard;

    
    if(isSquareEmpty(pos)) return 0ULL;

    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    int start = 0, end = 0;

    switch(pieceType){
        case Pawn:
            PieceAttacks = isPieceWhite(pos) ? whitePawnAttacks[pos] : blackPawnAttacks[pos];
            //The pawn can only attack (diagonally) is there is an enemy piece there
            PieceAttacks &= (isPieceWhite(pos) ? state.blackPieceBitBoard : state.whitePieceBitBoard);
            break;
        case Knight:
            PieceAttacks = KnightMoves[pos];
            break;
        case Bishop:
            PieceAttacks = BishopMoves[pos];
            start = 4;
            end = 8;
            break;
        case Rook:
            PieceAttacks = RookMoves[pos];
            start = 0;
            end = 4;
            break;
        case King:
            PieceAttacks = KingMoves[pos];
            break;
        case Queen:
            PieceAttacks = QueenMoves[pos];
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

        uint64_t blockers1Direction = blockers & Rays[directions[i]][pos];

        if(!blockers1Direction) continue;

        int firstBlockerPos = getFirstBlocker(pos,directions[i]);
	
	if(firstBlockerPos == -1) continue;
        
        blockingRays |= Rays[directions[i]][firstBlockerPos];    
    
    }


    uint64_t attacks = (PieceAttacks) & (~getFriendlyPieces(pos)) & (~blockingRays);

    if(pieceType == Pawn && state.enPassantSquare != -1){
        
        bool pieceWraps = pieceWrapsTheBoard(state.enPassantSquare,pos);

        //The pawn must be diagonally next to the enpassant square to move there
        if((isPieceWhite(pos) && ((state.enPassantSquare - 9 == pos) || (state.enPassantSquare - 7 == pos))) && !pieceWraps){

            attacks |= (1ULL << state.enPassantSquare);
        }else if(!isPieceWhite(pos) && ((state.enPassantSquare + 9 == pos) || (state.enPassantSquare + 7 == pos))&& !pieceWraps){

            attacks |= (1ULL << state.enPassantSquare);
        }
 
    }

    return attacks;

}

uint64_t Board::getAllAttacks(bool isWhite){

    uint64_t attacks = 0ULL;
    uint64_t pieceBitBoard = isWhite ? state.whitePieceBitBoard : state.blackPieceBitBoard;

    
    while(pieceBitBoard){

	int location = __builtin_ctzll(pieceBitBoard);
	pieceBitBoard &= (pieceBitBoard-1);

        attacks |= getAttacks(location);
    }

    return attacks;
}

uint64_t Board::getAttackers(int pos, bool attackingIsWhite){

    uint64_t attackers = 0ULL;

    attackers |= getPawnAttackers(pos, attackingIsWhite);

    attackers |= (attackingIsWhite ? (KnightMoves[pos] & state.whiteKnightBitBoard) : (KnightMoves[pos] & state.blackKnightBitBoard));

    for (auto &[direction,offset] : Compass){

        int attackingPiecePos = getFirstBlocker(pos, direction);
	
	if(attackingPiecePos == -1) continue;

        Pieces pieceType = (Pieces)getPieceEnum(attackingPiecePos);

        if(isPieceWhite(attackingPiecePos) != attackingIsWhite) continue;

        bool straightSliding = (direction == North || direction == East || direction == West || direction == South);

        if((straightSliding && ((pieceType == Rook) || pieceType == Queen)) || 
          (!straightSliding && ((pieceType == Bishop || pieceType == Queen)))){

            attackers |= (1ULL << attackingPiecePos);
        }
    }

    return attackers;

}

uint64_t Board::getPawnAttackers(int pos, bool attackingIsWhite){

    if(!posInBounds(pos)) return 0ULL;

    uint64_t attackers = 0ULL;
    uint64_t *pieceBitBoard = attackingIsWhite ? &state.whitePawnBitBoard : &state.blackPawnBitBoard;

    int offsets[2] = {7,9};

    for(int direction : offsets){

        int pawnPos = attackingIsWhite ? pos-direction : pos+direction;

        bool pieceWraps = pieceWrapsTheBoard(pos, pawnPos);

        if((*pieceBitBoard & (1ULL << pawnPos)) && !pieceWraps){

            attackers |= (1ULL << pawnPos);

        }
    }

    return attackers;

}

uint64_t Board::pawnControlledSquare(bool controllingColourIsWhite){

    uint64_t pawnBitBoard = controllingColourIsWhite ? state.whitePawnBitBoard : state.blackPawnBitBoard;
    uint64_t *pawnAttacks = controllingColourIsWhite ? whitePawnAttacks : blackPawnAttacks;
    uint64_t controlledSquares = 0ULL;
    
    while(pawnBitBoard){
        int pawnPos = __builtin_ctzll(pawnBitBoard);
        controlledSquares |= pawnAttacks[pawnPos];
        pawnBitBoard ^= (1ULL << pawnPos);
    }

    return controlledSquares;

}

uint64_t Board::getFriendlyPieces(int pos){
    return isPieceWhite(pos) ? state.whitePieceBitBoard : state.blackPieceBitBoard;
}

uint64_t Board::getKingLocation(bool isWhite){
    return isWhite ? __builtin_ctzll(state.whiteKingBitBoard) : __builtin_ctzll(state.blackKingBitBoard);
}

uint64_t Board::getEnemyPieces(int pos){
    return isPieceWhite(pos) ? state.blackPieceBitBoard : state.whitePieceBitBoard;
}

uint64_t Board::getRay(int pos1, int pos2){
    
    RaysDirection direction = convertPositionsToDirections(pos1,pos2);

    uint64_t ray = Rays[direction][pos1];
   
    ray &= ~Rays[direction][pos2];

    return ray;
}

uint64_t Board::getPinnedPieces(bool isWhite){

    uint64_t pinnedPieces = 0ULL;
    uint64_t kingLocation = getKingLocation(isWhite);
    
    
    for (auto const &[direction,offset] : Compass){

        uint64_t ray = Rays[direction][kingLocation];

        uint64_t pinnedPiece = ray & getFriendlyPieces(kingLocation);

        int firstBlocker = getFirstBlocker(kingLocation, direction);
        if(firstBlocker == -1) continue;

        //Pinned piece must be a friendly piece
        if(isPieceWhite(firstBlocker) != isPieceWhite(kingLocation))  continue;

        int secondBlocker = getFirstBlocker(firstBlocker,direction);
        if(secondBlocker == -1) continue;

        //If the second piece is also a friendly, nothing is pinned
        if(isPieceWhite(secondBlocker) == isPieceWhite(kingLocation)) continue;

        enum Pieces enemyPiece = (Pieces)getPieceEnum(secondBlocker);

        if((enemyPiece == Bishop || enemyPiece == Queen) && (direction == NorthEast || direction == NorthWest || direction == SouthEast || direction == SouthWest)){

            pinnedPieces |= (1ULL << firstBlocker);

        }else if((enemyPiece == Rook || enemyPiece == Queen) && (direction == North || direction == South || direction == East || direction == West)){

            pinnedPieces |= (1ULL << firstBlocker);

        }
         
    }

    return pinnedPieces;

}

uint64_t* Board::getBitBoardFromPiece(int pieceEnum, bool isWhite){
    uint64_t *pieceBitBoard = nullptr;
    switch(pieceEnum){
        case 0:
            pieceBitBoard = isWhite ? &state.whitePawnBitBoard : &state.blackPawnBitBoard;
            break;
        case 1:
            pieceBitBoard = isWhite ? &state.whiteRookBitBoard : &state.blackRookBitBoard;
            break;
        case 2:
            pieceBitBoard = isWhite ? &state.whiteBishopBitBoard : &state.blackBishopBitBoard;
            break;
        case 3:
            pieceBitBoard = isWhite ? &state.whiteKnightBitBoard : &state.blackKnightBitBoard;
            break;
        case 4:
            pieceBitBoard = isWhite ? &state.whiteQueenBitBoard : &state.blackQueenBitBoard;
            break;
        case 5:
            pieceBitBoard = isWhite ? &state.whiteKingBitBoard : &state.blackKingBitBoard;
            break;
        default:
            throw std::runtime_error("Invalid piece enum");
            break;
    }
    return pieceBitBoard;
}



int Board::getPieceEnum(int pos){

    uint64_t target = (1ULL << pos);

    if((state.whitePawnBitBoard & target) || (state.blackPawnBitBoard & target)){
        return 0;
    }
    else if ((state.whiteRookBitBoard & target)||(state.blackRookBitBoard & target)){
        return 1;
    }
    else if ((state.whiteBishopBitBoard & target)||(state.blackBishopBitBoard & target)){
        return 2;
    }
    else if ((state.whiteKnightBitBoard & target)||(state.blackKnightBitBoard & target)){
        return 3;
    }
    else if ((state.whiteQueenBitBoard & target)||(state.blackQueenBitBoard & target)){
        return 4;
    }
    else if ((state.whiteKingBitBoard & target)||(state.blackKingBitBoard & target)){
        return 5;
    }else{
        return -1;
    }

}

int Board::getFirstBlocker(int pos, RaysDirection direction){
    
    uint64_t blockers = Rays[direction][pos] & (state.whitePieceBitBoard | state.blackPieceBitBoard);

    if(!blockers) return -1; 

    if(direction == North || direction == East || direction == NorthEast || direction == NorthWest) return __builtin_ctzll(blockers);
         
    return 63 - __builtin_clzll(blockers);
    
}



bool Board::isAdversaryTurn(){

    return state.whiteToMove ? isAdversaryWhite : !isAdversaryWhite;
    
}

bool Board::isPieceWhite(int pos){
    return (1ULL << pos) & state.whitePieceBitBoard;
}

bool Board::isSquareEmpty(int pos){

    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.whitePieceBitBoard | state.blackPieceBitBoard;

    
    return !(target & allPieces);

}

bool Board::isSquareAttacked(int pos, bool attackingColourIsWhite){
    //return (1ULL << pos & getAllAttacks(attackingColourIsWhite));
    if (getPawnAttackers(pos, attackingColourIsWhite)) return true;

    uint64_t enemyKnights = attackingColourIsWhite ? state.whiteKnightBitBoard : state.blackKnightBitBoard;
    uint64_t enemyKing    = attackingColourIsWhite ? state.whiteKingBitBoard   : state.blackKingBitBoard;
    uint64_t enemyRooks   = attackingColourIsWhite ? state.whiteRookBitBoard   : state.blackRookBitBoard;
    uint64_t enemyBishops = attackingColourIsWhite ? state.whiteBishopBitBoard : state.blackBishopBitBoard;
    uint64_t enemyQueens  = attackingColourIsWhite ? state.whiteQueenBitBoard  : state.blackQueenBitBoard;

    if(KnightMoves[pos] & enemyKnights) return true;
    if(KingMoves[pos] & enemyKing) return true;
    
    RaysDirection diagonals[4] = {NorthEast,NorthWest,SouthEast,SouthWest};
    RaysDirection straights[4] = {North,South,East,West};

    for(RaysDirection direction : diagonals){

        int attackerPos = getFirstBlocker(pos, direction);
        if(attackerPos == -1) continue;

        if((1ULL << attackerPos) & (enemyBishops | enemyQueens)) return true;

    }

    for(RaysDirection direction : straights){

         int attackerPos = getFirstBlocker(pos, direction);
         if(attackerPos == -1) continue;

         if((1ULL << attackerPos) & (enemyRooks | enemyQueens)) return true;
    }

    return false;

}




void Board::updatePieceBitBoards(){

    state.whitePieceBitBoard = state.whitePawnBitBoard | state.whiteBishopBitBoard | state.whiteKnightBitBoard | state.whiteQueenBitBoard | state.whiteKingBitBoard | state.whiteRookBitBoard;
    state.blackPieceBitBoard = state.blackPawnBitBoard | state.blackBishopBitBoard | state.blackKnightBitBoard | state.blackQueenBitBoard | state.blackKingBitBoard | state.blackRookBitBoard;

}

void Board::makeMove(int from, int to, int promotionPieceType){

    Pieces pieceType = (Pieces)getPieceEnum(from);
    bool isWhite = isPieceWhite(from);

    uint64_t *pieceBitBoard = getBitBoardFromPiece((int)pieceType,isWhite);


    if((1ULL << to) & (state.whitePieceBitBoard | state.blackPieceBitBoard)){
        //Capturing a piece
        uint64_t *capturedBitBoard = getBitBoardFromPiece(getPieceEnum(to),!isWhite);

        //Toggle off the captured piece
        *capturedBitBoard ^= (1ULL << to);

        state.halfMoveClock = -1;
    }

    else if(pieceType == Pawn && to == state.enPassantSquare && state.enPassantSquare != -1){
        uint64_t *capturedBitBoard = getBitBoardFromPiece(Pawn,!isWhite);
        int capturePos = isWhite ? to - 8 : to + 8;
        *capturedBitBoard ^= (1ULL << capturePos);
        state.halfMoveClock = -1;
    }
    *pieceBitBoard |= (1ULL << to);
    *pieceBitBoard ^= (1ULL << from);

    if((pieceType == Pawn)){

        if(abs(convertLocationToRows(from) - convertLocationToRows(to)) == 2){
            //Set the enpassant square
            state.enPassantSquare = isWhite ? from+8 : from-8;
            
        }else{
            state.enPassantSquare = -1;
        }

        if(isWhite && convertLocationToRows(to) == 7){
            //Pawn promotes
            uint64_t *newPieceBitBoard = getBitBoardFromPiece(promotionPieceType,isWhite);
            *newPieceBitBoard |= (1ULL << to);
            *pieceBitBoard ^= (1ULL << to);
        }

        state.halfMoveClock = -1;
        
        
    }else{
        state.enPassantSquare = -1;
    }

    if(pieceType == King && state.castlingRights > 0){

        if(isWhite){
            state.castlingRights ^= 12;
        }else{
            state.castlingRights ^= 3;
        }

    }if(pieceType == Rook && state.castlingRights > 0){
        //Remove coresponding castling letter
        if(isWhite && from == 0) state.castlingRights &= ~4;
        if(isWhite && from == 7) state.castlingRights &= ~8;
        if(!isWhite && from == 63) state.castlingRights &= ~2;
        if(!isWhite && from == 56) state.castlingRights &= ~1;
    }

    updatePieceBitBoards();


    if(state.whiteToMove != state.whiteStarts) state.fullMoveClock++;
    state.halfMoveClock++;

    state.whiteToMove = !state.whiteToMove;
    
}

void Board::parseFenString(std::string fen){

    uint64_t boardIndex = 56;

    for(int i = 0; i < fen.length(); i++){

        char character = fen[i];
        char uCharacter = static_cast<unsigned char>(character);


        if(std::isdigit(uCharacter)){
            boardIndex += (uCharacter - '0');
            continue;

        }else if(std::isalpha(uCharacter)){

            uint64_t *pieceBitBoard = nullptr;

            bool isWhite = std::isupper(uCharacter);

            switch (std::tolower(uCharacter)) {
                case 'r':
                    pieceBitBoard = isWhite ? &state.whiteRookBitBoard : &state.blackRookBitBoard;
                    break;
                case 'n':
                    pieceBitBoard = isWhite ? &state.whiteKnightBitBoard : &state.blackKnightBitBoard;
                    break;
                case 'b':
                    pieceBitBoard = isWhite ? &state.whiteBishopBitBoard : &state.blackBishopBitBoard;
                    break;
                case 'q':
                    pieceBitBoard = isWhite ? &state.whiteQueenBitBoard : &state.blackQueenBitBoard;
                    break;
                case 'k':
                    pieceBitBoard = isWhite ? &state.whiteKingBitBoard : &state.blackKingBitBoard;
                    break;
                case 'p':
                    pieceBitBoard = isWhite ? &state.whitePawnBitBoard : &state.blackPawnBitBoard;
                    break;
                default:
                    throw std::runtime_error("Unknown piece character in FEN.");
                        
            }

            *pieceBitBoard |= (1ULL << boardIndex);

        }else if(uCharacter == '/'){

            //Move position down a line and to the start (left) side
            boardIndex -= 16;
            
            continue;
        }else if(uCharacter == ' '){
            //End of piece data in fen
            fen = fen.substr(i+1);
            break;
        }

        boardIndex += 1;
            
    }

    updatePieceBitBoards();

    //Parse game data from the end of FEN string

    std::stringstream tokens(fen);
    std::string token;

    tokens >> token;
    state.whiteToMove = (token == "w");
    state.whiteStarts = (token == "w");

    tokens >> token;
    
    for(char c : token){
        
        switch(c){
            case('K'):
                state.castlingRights += 8;
                break;  
                
            case('Q'):
                state.castlingRights += 4;
                break;
            
            case('k'):
                state.castlingRights += 2;
                break;
            
            case('q'):
                state.castlingRights += 1;
                break;
            
        }
	} 


    tokens >> token;

    if(token == "-") state.enPassantSquare = -1;
    else state.enPassantSquare = convertNotationToInt(token);

    tokens >> state.halfMoveClock;
    tokens >> state.fullMoveClock;
}
