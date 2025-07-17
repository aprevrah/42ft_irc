#pragma once
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

#include "Client.hpp"
#include "Channel.hpp"
#include "ChannelManager.hpp"

#define SERVER_NAME "AmazingServer"
#define MAX_EVENTS 20

class Client;

class Server {
   private:
    int               server_socket_fd;
    int               epoll_fd;
    const int         port;
    const std::string password;
    // the key is the filediscriptor on which the client is connected
    std::map<int, Client> clients;

    void handle_new_connection();
    void handle_received_data(int fd);

   public:
    ChannelManager chan_man;
    Server(const int port, const std::string password);
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();
    void run();
    bool is_correct_password(std::string password);

    bool is_nick_available(const std::string nick);
};
