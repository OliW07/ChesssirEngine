#include "log.h"
#include <iostream>
#include <fstream>
#include <sstream>


void log_uci(const std::string &msg, std::mutex &uci_mutex){

    static std::ofstream uci_log_file("uci_log.txt");
    if (uci_log_file.is_open()) {
        uci_log_file << msg << std::endl;
    }

    std::lock_guard<std::mutex> lock(uci_mutex);
    std::cout << msg << std::endl;

}

void log_uci(const int depth, const int bestScore, const long long nodesVisited,  Move pv, std::chrono::time_point<std::chrono::steady_clock> startTime, std::mutex &uci_mutex) {

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    long long nps = (elapsed > 0) ? (nodesVisited * 1000 / elapsed) : 0;

    std::stringstream log;
    log << "info depth " << depth 
             << " score cp " << bestScore
             << " nodes " << nodesVisited
             << " nps " << nps
             << " pv " << convertMoveToAlgebraicNotation(pv);
       
    log_uci(log.str(), uci_mutex);

}
