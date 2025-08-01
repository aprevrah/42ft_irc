#include "Client.hpp"

#include <cstring>
#include <iostream>

#include "Server.hpp"

class Command;

Client::Client() : server(NULL), fd(-1), registered(false), correct_password(false) {}

Client::Client(int fd, Server* server)
    : server(server), fd(fd), hostname("localhost"), registered(false), correct_password(false) {}

Client::Client(const Client& other) : server(other.server), fd(other.fd) {
    this->nickname = other.nickname;
    this->hostname = other.hostname;
    this->message_buffer = other.message_buffer;
    this->registered = other.registered;
    this->correct_password = other.correct_password;
}

Client& Client::operator=(const Client& other) {
    this->fd = other.fd;
    this->server = other.server;
    this->nickname = other.nickname;
    this->hostname = other.hostname;
    this->message_buffer = other.message_buffer;
    this->registered = other.registered;
    this->correct_password = other.correct_password;
    return *this;
}

Client::~Client() {}

t_command_status Client::add_to_buffer(std::string new_bytes) {
    message_buffer += new_bytes;

    // Check for complete messages (ending with CRLF)
    size_t crlf_pos;
    while ((crlf_pos = message_buffer.find("\r\n")) != std::string::npos) {
        // Extract complete message (without CRLF)
        std::string complete_message = message_buffer.substr(0, crlf_pos);
        log_msg(INFO, "\033[96mclient " + to_string(fd) + " ➤ '" + complete_message + "'\033[0m");

        try {
            Command cmd(complete_message, *this);
            if (cmd.execute(this->server) == CLIENT_DISCONNECTED) {
                return CLIENT_DISCONNECTED;
            }
        } catch (std::exception& e) {
            log_msg(ERROR, "Error while parsing command: " + std::string(e.what()));
        }
        message_buffer.erase(0, crlf_pos + 2);
    }
    return CMD_SUCCESS;
}

bool Client::is_registered() {
    return registered;
}

bool Client::try_register() {
    if (registered) {
        return false;
    }
    if (nickname.empty() || username.empty()) {
        return false;
    }
    if (!correct_password) {
        send_numeric_response(ERR_PASSWDMISMATCH, std::string(), "Password incorrect");
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

void Client::send_response(const std::string& response) const {
    std::string formatted_response = response;
    log_msg(INFO, "\033[92mclient " + to_string(fd) + " ⮜ '" + response + "'\033[0m");
    formatted_response += "\r\n";
    write(this->fd, formatted_response.c_str(), formatted_response.length());
}

void Client::send_numeric_response(const unsigned int numeric, std::string params, const std::string& message) const {
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