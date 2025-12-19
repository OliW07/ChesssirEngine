#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <atomic>

#include "utils/Types.h"
#include "board.h"


class Engine {
    public:


        long long nodesVisited = 0;
        int timeToThink;
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        std::atomic<bool> stopRequested = false;

        void search(Game game); //Copy of game to ensure 
        void setTimeToThink(Game &game);
        bool abortSearch(Game &game);
        int miniMax(Game &game, int maxDepth, int alpha, int beta);

};
#endif
