#ifndef LOG_H
#define LOG_H

#include "Types.h"
#include <mutex>

void log_uci(const std::string &msg, std::mutex &uci_mutex);
void log_uci(const int depth, const int bestScore, const int nodesVisited,  Move pv, std::chrono::time_point<std::chrono::steady_clock> startTime, std::mutex &uci_mutex);

#endif
