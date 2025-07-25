#pragma once
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#define LOG_LEVEL DEBUG

typedef enum log_level {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
} t_log_level;

// printing on stderr because stdout could be redirected
void log_msg(t_log_level level, std::string msg) {
    if (level <= LOG_LEVEL) {
        std::time_t now = std::time(0);
        std::tm*    local_time = std::localtime(&now);
        char timestamp[32];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);
        std::cerr << "[" << timestamp << "] ";

        if (level == ERROR) {
            std::cerr << "\033[41;37m ERORR \033[0m ";
        } else if (level == WARNING) {
            std::cerr << "\033[45;37m WARNING \033[0m ";
        } else if (level == INFO) {
            std::cerr << "\033[47;90m INFO \033[0m ";
        } else if (level == DEBUG) {
            std::cerr << "\033[103;90m DEBUG \033[0m ";
        }
        std::cerr << msg << std::endl;
    }
}