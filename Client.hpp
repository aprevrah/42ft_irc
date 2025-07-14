#pragma once
#include <string>
#include <string.h>
#include <cstring>
#include "Command.hpp"
#define MESSAGE_BUFFER_SIZE 512

class Client {
   private:
    int         fd;
    // char        message_buffer[MESSAGE_BUFFER_SIZE + 1];
    std::string message_buffer;
    std::string nickname;

   public:
    Client();
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();
    void add_to_buffer(std::string new_bytes);
    int  get_fd() const;
};
