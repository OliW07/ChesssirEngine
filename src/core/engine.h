#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <atomic>

#include "utils/Types.h"

class Game;

class Engine {
    private:

        Game &game;
    public:

        Engine(Game &gameParam) : game(gameParam) {};

        long long nodesVisited = 0;
        int timeToThink;
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        std::atomic<bool> stopRequested = false;

        Move search();
        void writeBestMove();
        void setTimeToThink();
        bool abortSearch();
        int miniMax(int maxDepth, int alpha, int beta);

};
#endif
