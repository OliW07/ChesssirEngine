#ifndef PERFT_H
#define PERFT_H

#include <map>
#include <cstdint>

#include "utils/Types.h"

class Board;

extern int maximum;

uint64_t perftSearch(Board &boardInstance, int maxDepth = maximum);
extern std::map<std::string,int> moveBreakDown;


#endif
