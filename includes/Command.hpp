#pragma once
#include <ostream>
#include <string>
#include <vector>

#include "ChannelManager.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "ft_irc.hpp"
#include "to_string.hpp"

class Server;
class Client;

class Command {
   private:
    std::string              prefix;
    std::string              command;
    std::vector<std::string> parameters;
    Client&                  client;

    t_command_status cmd_cap(Server* server);
    t_command_status cmd_pass(Server* server);
    t_command_status cmd_nick(Server* server);
    t_command_status cmd_user(Server* server);
    t_command_status cmd_ping(Server* server);
    t_command_status cmd_join(Server* server);
    t_command_status cmd_part(Server* server);
    t_command_status cmd_privmsg(Server* server);
    t_command_status cmd_quit(Server* server);
    t_command_status cmd_mode(Server* server);
    t_command_status cmd_invite(Server* server);
    t_command_status cmd_topic(Server* server);
    t_command_status cmd_kick(Server* server);

    int join_chan(Server* server, const std::string& chan_name, const std::string& key);

    void send_channel_modes(Channel* channel);
    void process_modes(Server* server, Channel* channel, const std::string& modes,
                       const std::vector<std::string>& params);

   public:
    Command(std::string command_str, Client& client);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();

    t_command_status execute(Server* server);

    friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
};
