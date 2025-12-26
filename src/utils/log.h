#ifndef LOG_H
#define LOG_H

#include <string>
#include <mutex>

void log_uci(const std::string& msg, std::mutex& uci_mutex);

#endif
