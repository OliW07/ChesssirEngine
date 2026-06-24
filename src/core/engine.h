#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

#include "Types.h"
#include "transpositionTable.h"

class Game;

class Engine {
    private:
    Game& game;

    public:
    Engine(Game& gameParam) : game(gameParam) {}

    long long nodesVisited = 0;
    int searchAge = 0;
    int timeToThink;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::atomic<bool> stopRequested = false;
    mutable std::mutex uci_mutex;
    TranspositionTable tt{512};

    Move search();
    void writeBestMove();
    void setTimeToThink();
    bool abortSearch();
    int negamax(int maxDepth, int alpha, int beta, int ply);
    int quiescence(int alpha, int beta, int ply);
};
