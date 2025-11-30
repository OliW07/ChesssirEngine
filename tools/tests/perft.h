#ifndef PERFT_H
#define PERFT_H

#include <map>


extern int maximum;

int perftSearch(Board &boardInstance, int maxDepth = maximum);
extern std::map<std::string,int> moveBreakDown;


#endif