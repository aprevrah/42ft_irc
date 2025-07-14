#include "Client.hpp"

#include <bsd/string.h>
#include <string.h>

#include <cstring>
#include <iostream>

Client::Client() {
    std::cout << "Client: Default constructor called" << std::endl;
}

Client::Client(int fd) : fd(fd) {
    std::cout << "Client: Parameter constructor called" << std::endl;
}

Client::Client(const Client& other) : fd(other.fd) {
    std::cout << "Client: Copy constructor called" << std::endl;
    this->nickname = other.nickname;
    this->message_buffer = other.message_buffer;
}

Client& Client::operator=(const Client& other) {
    std::cout << "Client: Copy assignment operator called" << std::endl;
    this->fd = other.fd;
    this->nickname = other.nickname;
    this->message_buffer = other.message_buffer;
    return *this;
}

Client::~Client() {
    std::cout << "Client: Destructor called" << std::endl;
}

void Client::add_to_buffer(std::string new_bytes) {
    message_buffer += new_bytes;

    // Check for complete messages (ending with CRLF)
    size_t crlf_pos;
    while ((crlf_pos = message_buffer.find("\r\n")) != std::string::npos) {
        // Extract complete message (without CRLF)
        std::string complete_message = message_buffer.substr(0, crlf_pos);
        std::cout << "Complete message: " << complete_message << std::endl;

        try {
            Command cmd(complete_message);
            std::cout << cmd << std::endl;
        } catch (std::runtime_error e) {
            std::cout << "Error while parsing command: " << e.what() << std::endl;
        }

        message_buffer.erase(0, crlf_pos + 2);
    }
}

int Client::get_fd() const {
    return fd;
}
