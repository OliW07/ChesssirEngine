#pragma once

#include <cstdint>
#include <map>

class Board;
class Game;

extern int maximum;
extern std::map<std::string, int> moveBreakDown;

uint64_t perftSearch(Game& game, int maxDepth = maximum);
extern std::map<std::string, int> moveBreakDown;
