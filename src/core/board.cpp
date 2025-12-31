#include <cstdlib>
#include <bit>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

#include "evaluate.h"
#include "fenHelper.h"
#include "board.h"
#include "Types.h"
#include "precompute.h"
#include "utils/Types.h"
#include "debug.h"
#include "zobrist.h"

using namespace precomputedData;


void Board::init(){
    initZobristKeys();
    precomputeBitBoardMoves();
}

uint64_t Board::getFriendlyPieces(int pos){
    return isPieceWhite(pos) ? state.occupancy[White]: state.occupancy[Black];
}

uint64_t Board::getKingLocation(bool isWhite){
    return isWhite ? __builtin_ctzll(state.bitboards[White][King]) : __builtin_ctzll(state.bitboards[Black][King]);
}

uint64_t Board::getEnemyPieces(int pos){
    return isPieceWhite(pos) ? state.occupancy[Black]: state.occupancy[White];
}

uint64_t Board::getRay(int pos1, int pos2){
    
    RaysDirection direction = convertPositionsToDirections(pos1,pos2);

    uint64_t ray = rays[direction][pos1];
   
    ray &= ~rays[direction][pos2];

    return ray;
}

uint64_t* Board::getBitBoardFromPiece(int pieceEnum, bool isWhite){
    uint64_t *pieceBitBoard = nullptr;
    switch(pieceEnum){
        case 0:
            pieceBitBoard = isWhite ? &state.bitboards[White][Bishop] : &state.bitboards[Black][Bishop];
            break;
        case 1:
            pieceBitBoard = isWhite ? &state.bitboards[White][Queen] : &state.bitboards[Black][Queen];
            break;
        case 2:
            pieceBitBoard = isWhite ? &state.bitboards[White][Rook] : &state.bitboards[Black][Rook];
            break;
        case 3:
            pieceBitBoard = isWhite ? &state.bitboards[White][King] : &state.bitboards[Black][King];
            break;
        case 4:
            pieceBitBoard = isWhite ? &state.bitboards[White][Pawn] : &state.bitboards[Black][Pawn];
            break;
        case 5:
            pieceBitBoard = isWhite ? &state.bitboards[White][Knight] : &state.bitboards[Black][Knight];
            break;
        default:
            throw std::runtime_error("Invalid piece enum");
            break;
    }
    return pieceBitBoard;
}



void Board::resetPosition(){
    BoardState newState;
    state = newState;
    
    isAdversaryWhite = false;
    historyIndex = 0;
    
    for(int i = 0; i < 2048; i++){
        history[i] = SavedData{};
    }
}

int Board::getPieceEnum(int pos){
      //Toggle off the colour of piece, to get left with the enum
      return state.mailBox[pos] & ~8; 

}

int Board::getFirstBlocker(int pos, RaysDirection direction){
    
    uint64_t blockers = rays[direction][pos] & (state.occupancy[White] | state.occupancy[Black]);

    if(!blockers) return -1; 

    if(direction == North || direction == East || direction == NorthEast || direction == NorthWest) return __builtin_ctzll(blockers);
         
    return 63 - __builtin_clzll(blockers);
    
}

void Game::setPosition(std::string fen,MoveList moves){
    parseFenString(fen,board.state);


    board.state.zhash = generateFullHash(board);

    for(auto &move : moves){
        board.makeMove(move);
    }

    board.isAdversaryWhite = !board.state.whiteToMove;
    
    setFullEval(board);
}



bool Board::isAdversaryTurn(){

    return state.whiteToMove ? isAdversaryWhite : !isAdversaryWhite;
    
}

bool Board::isPieceWhite(int pos){
    return (1ULL << pos) & state.occupancy[White];
}

bool Board::isSquareEmpty(int pos){

    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.occupancy[White] | state.occupancy[Black];

    
    return !(target & allPieces);

}

bool Game::isDraw(){
    
    return isTwoFoldRepition() || isInsufficientMaterial() || isFiftyMoveLimit();
}

bool Game::isTwoFoldRepition(){

    int maxHistoryDepth = std::max(0,board.historyIndex - board.state.halfMoveClock);

    //Start at the last index of the same side to move, only look at the same colour to move as we are checking for duplicates
    for(int i = board.historyIndex-2; i >= maxHistoryDepth; i-=2){
        
        if(board.state.zhash == board.history[i].zhash) return true;
    }

    return false;
}

bool Game::isInsufficientMaterial(){

    if (board.state.bitboards[Both][Queen] | board.state.bitboards[Both][Rook] | board.state.bitboards[Both][Pawn]) {
        return false;
    }

    int whiteKnights = std::popcount(board.state.bitboards[White][Knight]);
    int blackKnights = std::popcount(board.state.bitboards[Black][Knight]);
    int whiteBishops = std::popcount(board.state.bitboards[White][Bishop]);
    int blackBishops = std::popcount(board.state.bitboards[Black][Bishop]);

    int whiteMinors = whiteKnights + whiteBishops;
    int blackMinors = blackKnights + blackBishops;
    int totalMinors = whiteMinors + blackMinors;

    if(totalMinors < 2) return true;

    if (totalMinors == 2) {

        if (whiteKnights == 1 && blackKnights == 1) {
             return true;
        }
        
        if (whiteBishops == 1 && blackBishops == 1) {
            int whiteBLoc = __builtin_ctzll(board.state.bitboards[White][Bishop]);
            int blackBLoc = __builtin_ctzll(board.state.bitboards[Black][Bishop]);

            bool whiteBColor = getSquareColour(whiteBLoc); 
            bool blackBColor = getSquareColour(blackBLoc);
            
            // Draw only if bishops are on the same color
            return (whiteBColor == blackBColor);
        }
    }

    // Only a draw if the side with the knights is playing against a lone King.
    if (whiteMinors == 2 && whiteKnights == 2 && blackMinors == 0) return true;
    if (blackMinors == 2 && blackKnights ==2 && whiteMinors == 0) return true;

    return false;

}

bool Game::isFiftyMoveLimit(){
    return (board.state.halfMoveClock >= 50);
}

bool Board::isCapture(Move &move){
    return (state.occupancy[Both] & (1ULL << move.to));
}

int Board::scoreMove(Move &move){
    if(isCapture(move)){
        return (10 * PieceValues[getPieceEnum(move.to)]) - PieceValues[getPieceEnum(move.from)];
    }
    return 0;
}





