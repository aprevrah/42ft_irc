#pragma once
#include <string.h>

#include <cstring>
#include <string>

#include "Command.hpp"

class Server; // Forward declaration

#define MESSAGE_BUFFER_SIZE 512

class Client {
   private:
    Server*     server;
    int         fd;
    std::string message_buffer;
    std::string nickname;
    std::string username;
    bool        registered;

   public:
    bool correct_password;
    Client();
    Client(int fd, Server* server);
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();
    void add_to_buffer(std::string new_bytes);
    int  get_fd() const;
    
    void send_response(const std::string& response);

    const std::string &get_nickname() const;
    void set_nickname(const std::string &nickname);
    bool try_register();
    bool is_registered();
    void set_username(std::string);
};
