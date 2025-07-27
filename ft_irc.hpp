#pragma once
#include <string>
#include <iostream>

#define LOG_LEVEL DEBUG

typedef enum command_status {
    CMD_SUCCESS,
    CMD_FAILURE, // their was some error but the client is still connected
    CLIENT_DISCONNECTED, // if the client send QUIT or sth fatal goes wrong
} t_command_status;

typedef enum log_level {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
} t_log_level;

void log_msg(t_log_level level, std::string msg);