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

void log_msg(t_log_level level, std::string msg);