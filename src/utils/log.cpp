#include "log.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

void log(const LogType logType, const std::string& msg) {
    const File& file = logFiles[logType];
    std::lock_guard<std::mutex> lock(file.mutex);
    static std::ofstream logStream = [&file]() {
        std::ofstream fileStream(file.path);
        if (!fileStream.is_open()) {
            std::cerr << "[ERROR] failed to open the UCI log at " << file.path << "\n";
        }
        return fileStream;
    }();

    if (logStream.is_open()) {
        logStream << msg << std::endl;
    }
    std::cout << msg << std::endl;
}

void logUci(const int depth, const int bestScore, const long long nodesVisited, Move pv,
            std::chrono::time_point<std::chrono::steady_clock> startTime) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
    long long nps = (elapsed > 0) ? (nodesVisited * 1000000LL / elapsed) : 0;

    std::stringstream msg;
    msg << "info depth " << depth << " score cp " << bestScore << " nodes " << nodesVisited << " nps " << nps << " pv "
        << convertMoveToAlgebraicNotation(pv);

    log(LogType::UCI_LOG, msg.str());
}
