#include <ctime>

#include "ft_irc.hpp"

void print_timestamp() {
    std::time_t now = std::time(0);
    if (now == (time_t)-1) {
        return ;
    }
    std::tm* local_time = std::localtime(&now);
    if (local_time == NULL) {
        return ;
    }
    char timestamp[32];
    if (std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time) == 0) {
        return ;
    }
    std::cerr << "[" << timestamp << "] ";
}

// printing on stderr because stdout could be redirected
void log_msg(t_log_level level, std::string msg) {
    if (level <= LOG_LEVEL) {
        print_timestamp();
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