#include "log.h"
#include <iostream>
#include <fstream>
#include <mutex>

void log_uci(const std::string& msg, std::mutex& uci_mutex) {
    // Log to a dedicated UCI log file first.
    // The stream is static so it's initialized once and stays open.
    static std::ofstream uci_log_file("uci_log.txt");
    if (uci_log_file.is_open()) {
        uci_log_file << msg << std::endl;
    }

    // Then, send the same message to the actual standard output.
    {
        std::lock_guard<std::mutex> lock(uci_mutex);
        std::cout << msg << std::endl;
    }
}
