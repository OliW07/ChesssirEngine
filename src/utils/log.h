#pragma once

#include <filesystem>

#include "Types.h"

const std::filesystem::path UCI_LOG_PATH = std::filesystem::path(PROJECT_ROOT_DIR) / "logs" / "uci.log";

void log_uci(const std::string& msg);
void log_uci(const int depth, const int bestScore, const long long nodesVisited, Move pv,
             std::chrono::time_point<std::chrono::steady_clock> startTime);
