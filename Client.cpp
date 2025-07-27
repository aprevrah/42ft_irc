#include "Client.hpp"

#include <string.h>

#include <cstring>
#include <iostream>

#include "Server.hpp"

class Command;

Client::Client() {}

Client::Client(int fd, Server* server) : server(server), fd(fd), hostname("localhost"), registered(false) {}

Client::Client(const Client& other) : server(other.server), fd(other.fd) {
    this->nickname = other.nickname;
    this->hostname = other.hostname;
    this->message_buffer = other.message_buffer;
    this->registered = other.registered;
}

Client& Client::operator=(const Client& other) {
    this->fd = other.fd;
    this->server = other.server;
    this->nickname = other.nickname;
    this->hostname = other.hostname;
    this->message_buffer = other.message_buffer;
    return *this;
}

Client::~Client() {}

void Client::add_to_buffer(std::string new_bytes) {
    message_buffer += new_bytes;

    // Check for complete messages (ending with CRLF)
    size_t crlf_pos;
    while ((crlf_pos = message_buffer.find("\r\n")) != std::string::npos) {
        // Extract complete message (without CRLF)
        std::string complete_message = message_buffer.substr(0, crlf_pos);
        log_msg(INFO, "Message received: " + complete_message);

        try {
            Command cmd(complete_message, *this);
            message_buffer.erase(0, crlf_pos + 2);
            cmd.execute(this->server);
        } catch (std::exception& e) {
            message_buffer.erase(0, crlf_pos + 2);
            log_msg(ERROR, "Error while parsing command: " + std::string(e.what()));
        }

    }
}

bool  Client::is_registered() {
    return registered;
}

bool Client::try_register() {
    if (nickname.empty() || username.empty()) {
        return false;
    }
    if (!correct_password) {
        send_numeric_response(ERR_PASSWDMISMATCH, std::string(), "Password incorrect");
        // disconnect ?
        return false;
    }
    registered = true;
    send_numeric_response(001, "", "Welcome to the Internet Relay Network " + nickname + "!");
    send_numeric_response(002, "", "Your host is our.server42.at.");
    send_numeric_response(003, "", "This server was created today.");
    return true;
}

int Client::get_fd() const {
    return fd;
}

const std::string& Client::get_nickname() const {
    return this->nickname;
}

void Client::set_nickname(const std::string& nickname) {
    this->nickname = nickname;
}

void Client::send_response(const std::string& response) {
    std::string formatted_response = response;
    formatted_response += "\r\n";
    write(this->fd, formatted_response.c_str(), formatted_response.length());
}

// TODO: accept params ase multiple strings: maybe with array, vector or vargs
void Client::send_numeric_response(const unsigned int numeric, std::string params, const std::string& message) {
    std::string response = to_string(numeric);
    // make sure the numeric has 3 digits
    while (response.size() < 3) {
        response.insert(response.begin(), '0');
    }
    response += nickname.empty() ? " *" : (" " + nickname);
    if (!params.empty()) {
        response += " " + params;
    }
    response += " :" + message;
    send_response(response);
}

void Client::set_username(std::string new_name) {
    // TODO: validate username
    username = new_name;
}

const std::string& Client::get_hostname() const {
    return hostname;
}

std::string Client::get_prefix() const {
    return ":" + nickname + "!" + username + "@" + hostname;
}