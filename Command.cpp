#include "Command.hpp"

#include <iostream>
#include <stdexcept>

#include "Server.hpp"

// Command::Command() {
//     std::cout << "Command: Default constructor called" << std::endl;
// }

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
// this function expects that the CR LF is not in the string anymore
Command::Command(std::string command_str, Client& client) : client(client) {
    std::cout << "Command: Parameter constructor called" << std::endl;
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
    std::cout << "Command: Copy constructor called" << std::endl;
    *this = other;
}

Command& Command::operator=(const Command& other) {
    std::cout << "Command: Copy assignment operator called" << std::endl;
    this->prefix = other.prefix;
    this->command = other.command;
    this->parameters = other.parameters;
    return *this;
}

Command::~Command() {
    std::cout << "Command: Destructor called" << std::endl;
}

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
            os << "'" << cmd.parameters[i] << "'";
        }
    }
    return os;
}

void Command::cmd_cap(Server* server) {
    (void)server;
    if (parameters.size() > 0 && parameters.front() == "LS") {
        write(client.get_fd(), "CAP * LS :\r\n", 13);
    }
}

void Command::cmd_pass(Server* server) {
    if (parameters.size() < 1) {
        std::cout << "Not enough parameters" << std::endl;
        std::string response(ERR_NEEDMOREPARAMS);
        response += std::string(" PASS :Not enough parameters\r\n");
        write(client.get_fd(), response.c_str(), response.length());
        return ;
    }
    client.correct_password = server->is_correct_password(parameters.at(0));
    std::cout << "client.correct_password: " << client.correct_password << std::endl;
}

void Command::execute(Server* server) {
    (void)server;
    std::map<std::string, void (Command::*)(Server*)> cmd_functions;
    cmd_functions["CAP"] = &Command::cmd_cap;
    cmd_functions["PASS"] = &Command::cmd_pass;

    std::cout << "Executing command." << std::endl;
    if (cmd_functions.find(this->command) != cmd_functions.end()) {
        std::cout << "Command found: " << this->command << std::endl; 
        (this->*cmd_functions[this->command])(server);
    } else {
        std::cout << "Command not found: " << this->command << std::endl; 
    }
}
