#include "Client.hpp"
#include "Server.hpp"

#include <bsd/string.h>
#include <string.h>

#include <cstring>
#include <iostream>

class Command;

Client::Client() {
    std::cout << "Client: Default constructor called" << std::endl;
}

Client::Client(int fd, Server* server) :  server(server), fd(fd), registered(false) {
    std::cout << "Client: Parameter constructor called" << std::endl;
}

Client::Client(const Client& other) : server(other.server), fd(other.fd) {
    std::cout << "Client: Copy constructor called" << std::endl;
    this->nickname = other.nickname;
    this->message_buffer = other.message_buffer;
}

Client& Client::operator=(const Client& other) {
    std::cout << "Client: Copy assignment operator called" << std::endl;
    this->fd = other.fd;
    this->server = other.server;
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
            Command cmd(complete_message, *this);
            std::cout << cmd << std::endl;
            cmd.execute(this->server);
        } catch (std::exception &e) {
            std::cout << "Error while parsing command: " << e.what() << std::endl;
        }

        message_buffer.erase(0, crlf_pos + 2);
    }
}

bool Client::try_register() {
    if(nickname.empty() || username.empty()) {
        return false;
    }
    if (!correct_password) {
        write(fd, ERR_PASSWDMISMATCH "\r\n", 6);
        // disconnect ?
        return false;
    }
    registered = true;
    send_response("001 " + nickname + " :Welcome to the Internet Relay Network" + nickname + "!");
    send_response("002 " + nickname + " :Your host is our.server42.at.");
    send_response("003 " " :This server was created today.");
    return true;
}

int Client::get_fd() const {
    return fd;
}

const std::string &Client::get_nickname() const {
    return this->nickname;
}

void Client::set_nickname(const std::string &nickname) {
    this->nickname = nickname;
}

void Client::send_response(const std::string& response) {
    std::string formatted_response = response;
    formatted_response += "\r\n";
    write(this->fd, formatted_response.c_str(), formatted_response.length());
}
void Client::set_username(std::string new_name) {
    // TODO: validate username
    username = new_name;
}
