#pragma once
#include <ostream>
#include <string>
#include <vector>

#include "ChannelManager.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "to_string.hpp"

class Server;
class Client;

class Command {
   private:
    std::string              prefix;
    std::string              command;
    std::vector<std::string> parameters;
    Client&                  client;
    void                     cmd_cap(Server* server);
    void                     cmd_pass(Server* server);
    void                     cmd_nick(Server* server);
    void                     cmd_user(Server* server);
    void                     cmd_ping(Server* server);
    void                     cmd_join(Server* server);
    void                     cmd_part(Server* server);
    void                     cmd_privmsg(Server* server);

   public:
    Command(std::string command_str, Client& client);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();

    void execute(Server* server);

    friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
};
