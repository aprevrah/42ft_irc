#pragma once
#include <string>
#include <vector>
#include <ostream>
#include "Client.hpp"
#include "Server.hpp"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NONICKNAMEGIVEN "431"


#define ERR_NEEDMOREPARAMS "461"
#define ERR_PASSWDMISMATCH "464"

class Server;
class Client;

class Command {
   private:
        std::string                 prefix;
        std::string                 command;
        std::vector<std::string>    parameters;
        Client                      &client;
        void cmd_cap(Server* server);
        void cmd_pass(Server* server);
        void cmd_nick(Server* server);
        void cmd_user(Server* server);
   public:
//     Command();
    Command(std::string command_str, Client &client);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();

   
    void execute(Server *server);
    
    friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
};


