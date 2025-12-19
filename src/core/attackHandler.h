#ifndef ATTACKHANDLER_H
#define ATTACKHANDLER_H

#include "utils/Types.h"

class Board;

class AttackHandler {

    public:
        
        Board &board;
        AttackHandler(Board &boardInstance) : board(boardInstance) {}

        bool isSquareAttacked(int pos, bool attackingColourIsWhite);
        
        uint64_t getAllAttacks(bool isWhite);
        uint64_t getAttacks(const int pos);
        uint64_t getPawnAttackers(int pos, bool attackingIsWhite);
        uint64_t getAttackers(int pos, bool attackingIsWhite);
        uint64_t pawnControlledSquare(bool controllingColourIsWhite);
        uint64_t getPinnedPieces(bool isWhite, bool includeEnemies=false);
};



#endif
