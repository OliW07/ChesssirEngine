#pragma once

#include <cstdint>

class Board;
class Game;
enum class Colour : uint8_t;

using Square = uint8_t;

class AttackHandler {
    private:
    Game& game;

    public:
    AttackHandler(Game& gameParam) : game(gameParam) {};

    bool isSquareAttacked(Square square, Colour attackingColour);
    bool isCheck(Colour kingColour);

    uint64_t getAllAttacks(Colour colour);
    uint64_t getAttacks(const Square square);
    uint64_t getPawnAttackers(Square square, Colour attackingColour);
    uint64_t getAttackers(Square square, Colour attackingColour);
    uint64_t pawnControlledSquare(Colour controllingColour);
    uint64_t getPinnedPieces(Colour colour, bool includeEnemies = false);
};
