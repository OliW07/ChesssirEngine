#include <bitset>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "board.h"
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
extern std::unordered_map<std::string, uint64_t[64]> Rays;


Board::Board(const std::string fen, bool isAdversaryWhite){
    isAdversaryWhite = isAdversaryWhite;
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

    if(pieceType == King && state.castlingRights != "-"){

        for(char c : state.castlingRights){
            if(isupper(c) && isPieceWhite(pos)){
                if(c == 'K' && !isSquareAttacked(5,false) && !isSquareAttacked(6,false) && isSquareEmpty(5) && isSquareEmpty(6)){
                    //White kingside castling square
                    pseudoLegalMoves |= (1ULL << 6);
                }else if(c == 'Q' && !isSquareAttacked(3,false) && !isSquareAttacked(2,false) && !isSquareAttacked(1,false) && isSquareEmpty(3) && isSquareEmpty(2) && isSquareEmpty(1)){
                    //White queenside castling square
                    pseudoLegalMoves |= (1ULL << 2);
                }
            }else if(islower(c) && !isPieceWhite(pos)){
                if(c == 'k' && !isSquareAttacked(61,true) && !isSquareAttacked(62,true) && isSquareEmpty(61) && isSquareEmpty(62)){
                    //Black kingside castling square
                    pseudoLegalMoves |= (1ULL << 62);
                }else if(c == 'q' && !isSquareAttacked(57,true) && !isSquareAttacked(58,true) && !isSquareAttacked(59,true)&& isSquareEmpty(57) && isSquareEmpty(58) && isSquareEmpty(59)){
                    //Black queenside castling square
                    pseudoLegalMoves |= (1ULL << 58);
                }
                    
            }
        } 
        
    }


    return pseudoLegalMoves;

}

uint64_t Board::getFirstBlocker(int pos, std::string direction){
    
    uint64_t blockers = Rays[direction][pos] & (state.whitePieceBitBoard | state.blackPieceBitBoard);

    if(!blockers) return 0ULL;

    if(direction == "North" || direction == "East" || direction == "NorthEast" || direction == "NorthWest") return __builtin_ctzll(blockers);
         
    return 63 - __builtin_clzll(blockers);
    
}

uint64_t Board::getLegalMoves(const int pos){

    if(pos < 0 || pos > 63) throw std::runtime_error("The position provided, is not in range 0-63");


    uint64_t legalMoves = getPseudoLegalMoves(pos);

    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    //The king can't move to an attacked square
    if(pieceType == King) legalMoves &= ~getAllAttacks(!isPieceWhite(pos));
    
    //If the piece is pinned, limit its movement to the ray between itself and the king
    if(getPinnedPieces(isPieceWhite(pos)) & (1ULL << pos)){

        int kingLocation = getKingLocation(isPieceWhite(pos));

        std::string direction = convertPositionsToDirections(kingLocation,pos);

        legalMoves &= Rays[direction][kingLocation];
    }

    return legalMoves;

}

uint64_t Board::getAttacks(const int pos){

    uint64_t *PieceAttacks_p = nullptr;
    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.whitePieceBitBoard | state.blackPieceBitBoard;

    
    if(isSquareEmpty(pos)) return 0ULL;

    enum Pieces pieceType = (Pieces)getPieceEnum(pos);

    int start = 0, end = 0;

    switch(pieceType){
        case Pawn:
            PieceAttacks_p = isPieceWhite(pos) ? &whitePawnAttacks[pos] : &blackPawnAttacks[pos];
            //The pawn can only attack (diagonally) is there is an enemy piece there
            *PieceAttacks_p &= (isPieceWhite(pos) ? state.blackPieceBitBoard : state.whitePieceBitBoard);
            break;
        case Knight:
            PieceAttacks_p = &KnightMoves[pos];
            break;
        case Bishop:
            PieceAttacks_p = &BishopMoves[pos];
            start = 4;
            end = 8;
            break;
        case Rook:
            PieceAttacks_p = &RookMoves[pos];
            start = 0;
            end = 4;
            break;
        case King:
            PieceAttacks_p = &KingMoves[pos];
            break;
        case Queen:
            PieceAttacks_p = &QueenMoves[pos];
            start = 0;
            end = 8;
            break;
        default:
            throw std::runtime_error("Invalid piecetype: ");
            break;
        
    }

    //Find which piece it at the position

    uint64_t blockingRays = 0ULL;
    uint64_t blockers = *PieceAttacks_p & allPieces;

    const std::string directions[8] = {"North","East","South","West","NorthEast","SouthEast","SouthWest","NorthWest"};


    //calculate blockingRays in every direction for the different sliding pieces

    for(int i = start; i < end; i++){

        if(!blockers) break;

        uint64_t blockers1Direction = blockers & Rays[directions[i]][pos];

        if(!blockers1Direction) continue;

        int firstBlockerPos = getFirstBlocker(pos,directions[i]);
        
        blockingRays |= Rays[directions[i]][firstBlockerPos];    
    
    }


    uint64_t attacks = (*PieceAttacks_p) & (~getFriendlyPieces(pos)) & (~blockingRays);

    if(pieceType == Pawn && state.enPassantSquare != -1){
        
        int columnsDifference = abs(convertLocationToColumns(state.enPassantSquare) - convertLocationToColumns(pos));
        int rowsDifference = abs(convertLocationToRows(state.enPassantSquare) - convertLocationToRows(pos));

        bool pieceWrapsTheBoard = !((columnsDifference < 2) && (rowsDifference < 2));

        //The pawn must be diagonally next to the enpassant square to move there
        if((isPieceWhite(pos) && ((state.enPassantSquare - 9 == pos) || (state.enPassantSquare - 7 == pos))) && !pieceWrapsTheBoard){

            attacks |= (1ULL << state.enPassantSquare);
        }else if(!isPieceWhite(pos) && ((state.enPassantSquare + 9 == pos) || (state.enPassantSquare + 7 == pos))&& !pieceWrapsTheBoard){

            attacks |= (1ULL << state.enPassantSquare);
        }
 
    }

    return attacks;

}

uint64_t Board::getAllAttacks(bool isWhite){

    uint64_t attacks = 0ULL;
    std::vector<int> pieceLocations = isWhite ? getLocationsFromBitBoard(state.whitePieceBitBoard) : getLocationsFromBitBoard(state.blackPieceBitBoard);

    for(int location : pieceLocations){
        attacks |= getAttacks(location);
    }

    return attacks;
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
    
    std::string direction = convertPositionsToDirections(pos1,pos2);

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
        if(!firstBlocker) continue;

        //Pinned piece must be a friendly piece
        if(isPieceWhite(firstBlocker) != isPieceWhite(kingLocation))  continue;

        int secondBlocker = getFirstBlocker(firstBlocker,direction);
        if(!secondBlocker) continue;

        //If the second piece is also a friendly, nothing is pinned
        if(isPieceWhite(secondBlocker) == isPieceWhite(kingLocation)) continue;

        enum Pieces enemyPiece = (Pieces)getPieceEnum(secondBlocker);

        if((enemyPiece == Bishop || enemyPiece == Queen) && (direction == "NorthEast" || direction == "NorthWest" || direction == "SouthEast" || direction == "SouthWest")){

            pinnedPieces |= (1ULL << firstBlocker);

        }else if((enemyPiece == Rook || enemyPiece == Queen) && (direction == "North" || direction == "South" || direction == "East" || direction == "West")){

            pinnedPieces |= (1ULL << firstBlocker);

        }
         
    }

    return pinnedPieces;

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
    return (1ULL << pos & getAllAttacks(attackingColourIsWhite));
}




void Board::updatePieceBitBoards(){

    state.whitePieceBitBoard = state.whitePawnBitBoard | state.whiteBishopBitBoard | state.whiteKnightBitBoard | state.whiteQueenBitBoard | state.whiteKingBitBoard | state.whiteRookBitBoard;
    state.blackPieceBitBoard = state.blackPawnBitBoard | state.blackBishopBitBoard | state.blackKnightBitBoard | state.blackQueenBitBoard | state.blackKingBitBoard | state.blackRookBitBoard;

}

void Board::makeMove(int from, int to, int promotionPiece){

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

    tokens >> state.castlingRights;
    
    tokens >> token;

    if(token == "-") state.enPassantSquare = -1;
    else state.enPassantSquare = convertNotationToInt(token);

    tokens >> state.halfMoveClock;
    tokens >> state.fullMoveClock;
}

