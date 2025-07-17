#include "Command.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "Server.hpp"

// Helper function for C++98 compatibility (std::to_string not available)
std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string to_string(unsigned int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

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
            os << "'" << cmd.parameters[i] << "'";
        }
    }
    return os;
}

void Command::cmd_cap(Server* server) {
    (void)server;
    if (parameters.size() > 0 && parameters.front() == "LS") {
        client.send_response("CAP * LS :");
    }
}

void Command::cmd_nick(Server* server) {
    (void)server;
    std::string prev_nick = client.get_nickname().empty() ? "*" : client.get_nickname();
    if (parameters.size() > 0) {
        if (server->is_nick_available(parameters.front())) {
            client.set_nickname(parameters.front());
        } else {
            client.send_response(ERR_NICKNAMEINUSE" * " + parameters.front() + " :Nickname is already in use");
        }

        // std::string response = "TestResponse: Nick set to " + client.get_nickname();
        // client.send_response(response); //TODO: Check if nick is valid and error responses
        client.try_register();
    } else {
        client.send_response(ERR_NONICKNAMEGIVEN " " + prev_nick + " :No nickname given");
    }
}

void Command::cmd_pass(Server* server) {
    if (parameters.size() < 1) {
        std::cout << "Not enough parameters" << std::endl;
        std::string response(ERR_NEEDMOREPARAMS);
        response += std::string(" PASS :Not enough parameters\r\n");
        write(client.get_fd(), response.c_str(), response.length());
        return;
    }
    // check if already registered --> ERR_ALREADYREGISTRED
    client.correct_password = server->is_correct_password(parameters.at(0));
    std::cout << "client.correct_password: " << client.correct_password << std::endl;
}

// Parameters: <username> <hostname> <servername> <realname>
void Command::cmd_user(Server* server) {
    (void)server;
    if (parameters.size() < 4) {
        // TODO: send Error response
        return;
    }
    client.set_username(parameters.at(0));
    // ignore other paramters for now
    client.try_register();
}

void Command::cmd_ping(Server* server) {
    (void)server;
    if (parameters.size() > 0) {
        std::string &token = parameters.back();
        client.send_response("PONG " SERVER_NAME " " + token); //TODO: server name var?
    } else {
        client.send_response(ERR_NEEDMOREPARAMS);
    }
}

void Command::cmd_join(Server* server) { //TODO: join multiple channels
   if (parameters.size() > 0) {
        std::string &chan_name = parameters.front();
        try {
            server->chan_man.join_channel(&client, chan_name);
            client.send_response("JOIN " + chan_name); //TODO: correct msg
        } catch (IRCException& e) {
            client.send_response(to_string(e.get_irc_numeric()) + std::string(" ") + e.what()); //TODO: correct msg
        } catch (std::exception& e) {
            client.send_response(std::string("PART fail: ") + e.what());
        }
    }
}

void Command::cmd_part(Server* server) {
    if (parameters.size() > 0) {
        std::string &chan_name = parameters.front();
        try {
            server->chan_man.leave_channel(&client, chan_name);
            client.send_response("PART " + chan_name); //TODO: correct msg
        } catch (IRCException& e) {
            client.send_response(to_string(e.get_irc_numeric()) + std::string(" ") + e.what()); //TODO: correct msg
        } catch (std::exception& e) {
            client.send_response(std::string("PART fail: ") + e.what());
        }
    }
}

void Command::execute(Server* server) {
    (void)server;
    std::map<std::string, void (Command::*)(Server*)> cmd_functions;
    cmd_functions["CAP"] = &Command::cmd_cap;
    cmd_functions["PASS"] = &Command::cmd_pass;
    cmd_functions["NICK"] = &Command::cmd_nick;
    cmd_functions["USER"] = &Command::cmd_user;
    cmd_functions["PING"] = &Command::cmd_ping;
    cmd_functions["JOIN"] = &Command::cmd_join;
    cmd_functions["PART"] = &Command::cmd_part;

    if (cmd_functions.find(this->command) != cmd_functions.end()) {
        std::cout << "Command found: " << this->command << std::endl;
        (this->*cmd_functions[this->command])(server);
    } else {
        std::cout << "Command not found: '" << this->command << "'" << std::endl;
    }
}
