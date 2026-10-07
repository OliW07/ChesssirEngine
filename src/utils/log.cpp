#include "log.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

void log_uci(const std::string& msg) {
    static std::mutex uci_mutex;
    std::lock_guard<std::mutex> lock(uci_mutex);
    static std::ofstream uci_log_file = []() {
        std::ofstream file(UCI_LOG_PATH);
        if (!uci_log_file.is_open()) {
            std::cerr << "[ERROR] failed to open the UCI log at " << UCI_LOG_PATH << "\n";
        }
        return file;
    }();

    if (uci_log_file.is_open()) {
        uci_log_file << msg << std::endl;
    }
    std::cout << msg << std::endl;
}

void log_uci(const int depth, const int bestScore, const long long nodesVisited, Move pv,
             std::chrono::time_point<std::chrono::steady_clock> startTime) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
    long long nps = (elapsed > 0) ? (nodesVisited * 1000000LL / elapsed) : 0;

    std::stringstream log;
    log << "info depth " << depth << " score cp " << bestScore << " nodes " << nodesVisited << " nps " << nps << " pv "
        << convertMoveToAlgebraicNotation(pv);

    log_uci(log.str());
}
