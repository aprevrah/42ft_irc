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

typedef enum command_status {
    CMD_SUCCESS,
    CMD_FAILURE, // their was some error but the client is still connected
    CLIENT_DISCONNECTED, // if the client send QUIT or sth fatal goes wrong
} t_command_status;

class Command {
   private:
    std::string              prefix;
    std::string              command;
    std::vector<std::string> parameters;
    Client&                  client;
    t_command_status         cmd_cap(Server* server);
    t_command_status         cmd_pass(Server* server);
    t_command_status         cmd_nick(Server* server);
    t_command_status         cmd_user(Server* server);
    t_command_status         cmd_ping(Server* server);
    t_command_status         cmd_join(Server* server);
    t_command_status         cmd_part(Server* server);
    t_command_status         cmd_privmsg(Server* server);
    t_command_status         cmd_quit(Server* server);

   public:
    Command(std::string command_str, Client& client);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();

    t_command_status execute(Server* server);

    friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
};
