#pragma once

#include <cstdint>
#include <map>
#include <string>

class Game;

uint64_t perftSearch(Game& game, int maxDepth, int initialDepth,
                     std::map<std::string, int>* moveBreakDownOut = nullptr);
