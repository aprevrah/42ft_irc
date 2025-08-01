#include "Command.hpp"

#include <iostream>
#include <stdexcept>

#include "Server.hpp"

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
// this function expects that the CR LF is not in the string anymore
Command::Command(std::string command_str, Client& client) : client(client) {
    size_t current_pos = 0;
    size_t next_space = 0;

    // <prefix>
    if (command_str.size() != 0 && command_str.at(0) == ':') {
        current_pos = 1;
        next_space = command_str.find(' ', current_pos);
        prefix = command_str.substr(current_pos, next_space - current_pos);
        current_pos = command_str.find_first_not_of(' ', next_space);
    }

    // <command>
    next_space = command_str.find(' ', current_pos);
    command = command_str.substr(current_pos, next_space - current_pos);
    if (command.size() == 0) {
        throw std::runtime_error("Empty Command");
    }
    current_pos = command_str.find_first_not_of(' ', next_space);

    // <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
    // <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
    //                or NUL or CR or LF, the first of which may not be ':'>
    while (current_pos != std::string::npos) {
        next_space = command_str.find(' ', current_pos);
        if (command_str.size() > current_pos && command_str.at(current_pos) == ':') {  // <trailing>
            current_pos++;
            next_space = std::string::npos;
        }
        parameters.push_back(command_str.substr(current_pos, next_space - current_pos));
        current_pos = command_str.find_first_not_of(' ', next_space);
    }
}

Command::Command(const Command& other) : client(other.client) {
    *this = other;
}

Command& Command::operator=(const Command& other) {
    this->prefix = other.prefix;
    this->command = other.command;
    this->parameters = other.parameters;
    return *this;
}

Command::~Command() {}

std::ostream& operator<<(std::ostream& os, const Command& cmd) {
    os << "Command: " << cmd.command;
    if (!cmd.prefix.empty()) {
        os << " (prefix: " << cmd.prefix << ")";
    }
    if (!cmd.parameters.empty()) {
        os << " parameters: ";
        for (size_t i = 0; i < cmd.parameters.size(); ++i) {
            if (i > 0) {
                os << ", ";
            }
            os << "'" << cmd.parameters.at(i) << "'";
        }
    }
    return os;
}

t_command_status Command::execute(Server* server) {
    (void)server;
    std::map<std::string, t_command_status (Command::*)(Server*)> cmd_functions;
    cmd_functions["CAP"] = &Command::cmd_cap;
    cmd_functions["PASS"] = &Command::cmd_pass;
    cmd_functions["NICK"] = &Command::cmd_nick;
    cmd_functions["USER"] = &Command::cmd_user;
    cmd_functions["PING"] = &Command::cmd_ping;
    cmd_functions["JOIN"] = &Command::cmd_join;
    cmd_functions["QUIT"] = &Command::cmd_quit;
    cmd_functions["PART"] = &Command::cmd_part;
    cmd_functions["PRIVMSG"] = &Command::cmd_privmsg;
    cmd_functions["MODE"] = &Command::cmd_mode;
    cmd_functions["INVITE"] = &Command::cmd_invite;
    cmd_functions["TOPIC"] = &Command::cmd_topic;
    cmd_functions["KICK"] = &Command::cmd_kick;

    if (cmd_functions.find(this->command) != cmd_functions.end()) {
        log_msg(DEBUG, "Command found: " + this->command);
        return (this->*cmd_functions.at(this->command))(server);
    } else {
        log_msg(WARNING, "Command not found: " + this->command);
        return CMD_FAILURE;
    }
}
