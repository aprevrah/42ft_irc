#include "Client.hpp"

#include <string.h>

#include <iostream>

Client::Client(int fd) : fd(fd) {
    std::cout << "Client: Default constructor called" << std::endl;
}

Client::Client(const Client& other) : fd(other.fd) {
    std::cout << "Client: Copy constructor called" << std::endl;
}

Client& Client::operator=(const Client& other) {
    (void)other;
    std::cout << "Client: Copy assignment operator called" << std::endl;
    return *this;
}

Client::~Client() {
    std::cout << "Client: Destructor called" << std::endl;
}

void Client::add_to_buffer(char* new_bytes) {
    strlcat(message_buffer, new_bytes, MESSAGE_BUFFER_SIZE + 1);
}

int Client::get_fd() const {
    return fd;
}
