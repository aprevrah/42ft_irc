#pragma once
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

#include "Channel.hpp"
#include "ChannelManager.hpp"
#include "Client.hpp"
#include "ft_irc.hpp"

#define SERVER_NAME "AmazingServer"
#define MAX_EVENTS  20

class Client;

class Server {
   private:
    int               server_socket_fd;
    int               epoll_fd;
    const int         port;
    const std::string password;
    // the key is the filediscriptor on which the client is connected
    std::map<int, Client> clients;

    void        handle_new_connection();
    void        handle_received_data(int fd);
    void        run();
    static void signal_handler(int signal);

   public:
    ChannelManager chan_man;
    Server(const int port, const std::string password);
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();
    void start();
    bool is_correct_password(std::string password);
    void disconnect_client(int client_fd, std::string reason);
    void disconnect_all_clients();

    bool    is_nick_available(const std::string nick);
    Client* get_client_by_nick(const std::string nick);
};
