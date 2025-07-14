#include "Command.hpp"

#include <iostream>
#include <stdexcept>

Command::Command() {
    std::cout << "Command: Default constructor called" << std::endl;
}

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
// this function expects that the CR LF is not in the string anymore
Command::Command(std::string command_str) {
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

Command::Command(const Command& other) {
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
