#ifndef ATTACKHANDLER_H
#define ATTACKHANDLER_H

#include "utils/Types.h"

class Board;
class Game;

class AttackHandler {

    private:

        Game &game;
        
    public:
        
        AttackHandler(Game &gameParam) : game(gameParam){};

        bool isSquareAttacked(int pos, bool attackingColourIsWhite);
        bool isCheck(Colours kingColour);

        uint64_t getAllAttacks(bool isWhite);
        uint64_t getAttacks(const int pos);
        uint64_t getPawnAttackers(int pos, bool attackingIsWhite);
        uint64_t getAttackers(int pos, bool attackingIsWhite);
        uint64_t pawnControlledSquare(bool controllingColourIsWhite);
        uint64_t getPinnedPieces(bool isWhite, bool includeEnemies=false);
};



#endif
