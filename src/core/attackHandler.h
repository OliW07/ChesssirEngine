#pragma once

#include <cstdint>

class Board;
class Game;
enum class Colour : uint8_t;

class AttackHandler {
    private:
    Game& game;

    public:
    AttackHandler(Game& gameParam) : game(gameParam) {};

    bool isSquareAttacked(int pos, Colour attackingColour);
    bool isCheck(Colour kingColour);

    uint64_t getAllAttacks(Colour colour);
    uint64_t getAttacks(const int pos);
    uint64_t getPawnAttackers(int pos, Colour attackingColour);
    uint64_t getAttackers(int pos, Colour attackingColour);
    uint64_t pawnControlledSquare(Colour controllingColour);
    uint64_t getPinnedPieces(Colour colour, bool includeEnemies = false);
};
