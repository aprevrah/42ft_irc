
#include "Command.hpp"

#include <iostream>

Command::Command() {
    std::cout << "Command: Default constructor called" << std::endl;
}

Command::Command(std::string command_str) {
    std::cout << "Command: Parameter constructor called" << std::endl;
    // parse string
}

Command::Command(const Command& other) {
    std::cout << "Command: Copy constructor called" << std::endl;
}

Command& Command::operator=(const Command& other) {
    std::cout << "Command: Copy assignment operator called" << std::endl;
    return *this;
}

Command::~Command() {
    std::cout << "Command: Destructor called" << std::endl;
}

