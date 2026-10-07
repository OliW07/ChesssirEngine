#pragma once

#include <filesystem>
#include <mutex>

#include "Types.h"
#include "enumArray.h"

struct File {
    std::filesystem::path path;
    mutable std::mutex mutex;
    explicit File(std::filesystem::path p) : path(std::move(p)) {}
};

enum class LogType { UCI_LOG, DEBUG };
inline const EnumArray<File, LogType, 2> logFiles{std::filesystem::path(PROJECT_ROOT_DIR) / "logs" / "uci.log",
                                                  std::filesystem::path(PROJECT_ROOT_DIR) / "logs" / "debug.log"};

void log(const LogType logType, const std::string& msg);
void logUci(const int depth, const int bestScore, const long long nodesVisited, Move pv,
            std::chrono::time_point<std::chrono::steady_clock> startTime);
