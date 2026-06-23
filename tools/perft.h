#ifndef PERFT_H
#define PERFT_H

#include <cstdint>
#include <map>

#include "utils/Types.h"

class Board;
class Game;

extern int maximum;
extern std::map<std::string, int> moveBreakDown;

uint64_t perftSearch(Game& game, int maxDepth = maximum);
extern std::map<std::string, int> moveBreakDown;

#endif
