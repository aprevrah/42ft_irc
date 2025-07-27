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
            os << "'" << cmd.parameters[i] << "'";
        }
    }
    return os;
}

t_command_status Command::cmd_cap(Server* server) {
    (void)server;
    if (parameters.size() > 0 && parameters.front() == "LS") {
        client.send_response("CAP * LS :");
    }
    return CMD_SUCCESS;
}

t_command_status Command::cmd_nick(Server* server) {
    (void)server;
    if (parameters.size() > 0) {
        if (server->is_nick_available(parameters.front())) {
            client.set_nickname(parameters.front());
        } else {
            client.send_numeric_response(ERR_NICKNAMEINUSE, parameters.front(), "Nickname is already in use");
            return CMD_FAILURE;
        }

        // std::string response = "TestResponse: Nick set to " + client.get_nickname();
        // client.send_response(response); //TODO: Check if nick is valid and error responses
        client.try_register();
        return CMD_SUCCESS;
    } else {
        client.send_numeric_response(ERR_NONICKNAMEGIVEN, std::string(), "No nickname given");
        return CMD_FAILURE;
    }
}

t_command_status Command::cmd_pass(Server* server) {
    if (parameters.size() < 1) {
        log_msg(WARNING, "PASS command: Not enough parameters");
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "PASS", "Not enough parameters");
        return CMD_FAILURE;
    }
    if (client.is_registered()) {
        client.send_numeric_response(ERR_ALREADYREGISTERED, "PASS", "You may not reregister");
        return CMD_FAILURE;
    }
    // check if already registered --> ERR_ALREADYREGISTRED
    client.correct_password = server->is_correct_password(parameters.at(0));
    log_msg(DEBUG, "client.correct_password: " + to_string(client.correct_password));
    return CMD_SUCCESS;
}

// Parameters: <username> <hostname> <servername> <realname>
t_command_status Command::cmd_user(Server* server) {
    (void)server;
    if (parameters.size() < 4) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "USER", "Not enough parameters");
        return CMD_FAILURE;
    }
    if (client.is_registered()) {
        client.send_numeric_response(ERR_ALREADYREGISTERED, "USER", "You may not reregister");
    }
    client.set_username(parameters.at(0));
    // TODO: do sth with the other parameters?
    client.try_register();
    return CMD_SUCCESS;
}

t_command_status Command::cmd_ping(Server* server) {
    (void)server;
    if (parameters.size() > 0) {
        std::string& token = parameters.back();
        client.send_response("PONG " SERVER_NAME " " + token);  // TODO: server name var?
        return CMD_SUCCESS;
    } else {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    }
}

t_command_status Command::cmd_join(Server* server) {  // TODO: join multiple channels
    if (parameters.size() > 0) {
        std::string& chan_name = parameters.front();
        std::string  key = "";

        // Check if key is provided: JOIN #channel key
        if (parameters.size() > 1) {
            key = parameters[1];
        }

        try {
            server->chan_man.join_channel(&client, chan_name, key);

            std::string join_msg = client.get_prefix() + " JOIN " + chan_name;
            Channel*    channel = server->chan_man.find_channel_by_name(chan_name);
            if (channel) {
                channel->broadcast(join_msg, NULL);
            }
            return CMD_SUCCESS;
        } catch (IRCException& e) {
            client.send_numeric_response(e.get_irc_numeric(), chan_name, e.what());
            return CMD_FAILURE;
        } catch (std::exception& e) {
            client.send_response(std::string("JOIN fail: ") + e.what());
            return CMD_FAILURE;
        }
    }
    return CMD_FAILURE;
}

t_command_status Command::cmd_part(Server* server) {
    if (parameters.size() == 0) {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    } else {
        std::string& chan_name = parameters.front();
        try {
            server->chan_man.leave_channel(&client, chan_name);

            Channel* channel = server->chan_man.find_channel_by_name(chan_name);
            if (channel) {
                std::string part_msg = client.get_prefix() + " PART " + chan_name;
                channel->broadcast(part_msg, NULL);
            }
            return CMD_SUCCESS;
        } catch (IRCException& e) {
            client.send_response(to_string(e.get_irc_numeric()) + std::string(" ") + e.what());  // TODO: correct msg
            return CMD_FAILURE;
        } catch (std::exception& e) {
            client.send_response(std::string("PART fail: ") + e.what());
            return CMD_FAILURE;
        }
    }
}

t_command_status Command::cmd_quit(Server* server) {
    std::string reason = parameters.size() > 0 ? parameters.back() : "quit";
    server->disconnect_client(client.get_fd(), reason);
    return CLIENT_DISCONNECTED;
}

t_command_status Command::cmd_privmsg(Server* server) {
    if (parameters.size() != 2) {
        // TODO: what about to many params? is NEEDMOREPARAMS still the correct message?
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    }
    std::vector<std::string> targets = split_string(parameters[0], ',');
    std::string&             message = parameters[1];

    for (size_t i = 0; i < targets.size(); i++) {
        std::string& target = targets[i];
        if (target.empty()) {
            continue;
        }
        if (target.at(0) == '#') {
            Channel* channel = server->chan_man.find_channel_by_name(target);
            if (!channel) {
                // Channel does not exist
                client.send_numeric_response(ERR_NOSUCHCHANNEL, target, "No such channel");
                continue;
            }
            if (!channel->is_client_in_channel(&client)) {
                // Client not in channel - cannot send to channel
                client.send_numeric_response(ERR_CANNOTSENDTOCHAN, target, "Cannot send to channel");
                continue;
            }

            std::string privmsg = client.get_prefix() + " PRIVMSG " + target + " :" + message;
            channel->broadcast(privmsg, &client);
            continue;
        }

        Client* target_client = server->get_client_by_nick(target);
        if (!target_client) {
            client.send_numeric_response(ERR_NOSUCHNICK, target, "No such nick/channel");
            continue;
        }
        // Build the private message
        std::string privmsg = client.get_prefix() + " PRIVMSG " + target + " :" + message;
        server->chan_man.find_channel_by_name(target)->broadcast(privmsg, &client);
        continue;
    }
    return CMD_SUCCESS;
}

t_command_status Command::cmd_mode(Server* server) {
    if (parameters.size() < 1) {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    }

    std::string& target = parameters[0];

    // Only handle channel modes (starting with #)
    if (target[0] != '#') {
        // For user modes, just ignore (basic implementation)
        return CMD_FAILURE;
    }

    // Check if channel exists
    if (!server->chan_man.channel_exists(target)) {
        client.send_numeric_response(ERR_NOSUCHCHANNEL, target, "No such channel");
        return CMD_FAILURE;
    }

    Channel* channel = server->chan_man.find_channel_by_name(target);

    // Check if client is in channel
    if (!channel->is_client_in_channel(&client)) {
        client.send_numeric_response(ERR_NOTONCHANNEL, target, "You're not on that channel");
        return CMD_FAILURE;
    }

    // MODE #channel (query current modes)
    if (parameters.size() == 1) {
        send_channel_modes(channel);
        return CMD_FAILURE;
    }

    // Check if client is operator
    if (!channel->is_client_operator(&client)) {
        client.send_numeric_response(ERR_CHANOPRIVSNEEDED, target, "You're not channel operator");
        return CMD_FAILURE;
    }

    // MODE #channel +/-modes [parameters]
    std::string&             modes = parameters[1];
    std::vector<std::string> mode_params;

    // Collect mode parameters
    for (size_t i = 2; i < parameters.size(); i++) {
        mode_params.push_back(parameters[i]);
    }

    process_modes(server, channel, modes, mode_params);
    return CMD_SUCCESS;  // mode parsing could have failed
}

void Command::send_channel_modes(Channel* channel) {
    std::string mode_string = "+";
    std::string mode_params = "";

    // Build current modes
    if (channel->is_invite_only()) {
        mode_string += "i";
    }
    if (channel->is_topic_restricted()) {
        mode_string += "t";
    }
    if (channel->has_key()) {
        mode_string += "k";
        mode_params += " " + channel->get_key();
    }
    if (channel->has_user_limit()) {
        mode_string += "l";
        mode_params += " " + to_string(channel->get_user_limit());
    }

    // Send response: 324 RPL_CHANNELMODEIS
    std::string response = channel->get_name() + " " + mode_string + mode_params;
    client.send_numeric_response(324, response, "");
}

void Command::process_modes(Server* server, Channel* channel, const std::string& modes,
                            const std::vector<std::string>& params) {
    bool        adding = true;
    size_t      param_index = 0;
    std::string changes = "";
    std::string change_params = "";

    for (size_t i = 0; i < modes.length(); i++) {
        char mode = modes[i];

        if (mode == '+') {
            adding = true;
            continue;
        } else if (mode == '-') {
            adding = false;
            continue;
        }

        switch (mode) {
            case 'i':  // Invite-only
                if (adding != channel->is_invite_only()) {
                    channel->set_invite_only(adding);
                    changes += (adding ? "+" : "-");
                    changes += "i";
                }
                break;

            case 't':  // Topic restriction
                if (adding != channel->is_topic_restricted()) {
                    channel->set_topic_restricted(adding);
                    changes += (adding ? "+" : "-");
                    changes += "t";
                }
                break;

            case 'k':  // Channel key
                if (adding) {
                    if (param_index < params.size()) {
                        channel->set_key(params[param_index]);
                        changes += "+k";
                        if (!change_params.empty()) {
                            change_params += " ";
                        }
                        change_params += params[param_index];
                        param_index++;
                    }
                } else {
                    if (channel->has_key()) {
                        channel->set_key("");
                        changes += "-k";
                    }
                }
                break;

            case 'l':  // User limit
                if (adding) {
                    if (param_index < params.size()) {
                        int limit = atoi(params[param_index].c_str());
                        if (limit > 0) {
                            channel->set_user_limit(limit);
                            changes += "+l";
                            if (!change_params.empty()) {
                                change_params += " ";
                            }
                            change_params += params[param_index];
                        }
                        param_index++;
                    }
                } else {
                    if (channel->has_user_limit()) {
                        channel->set_user_limit(0);
                        changes += "-l";
                    }
                }
                break;

            case 'o':  // Operator
                if (param_index < params.size()) {
                    Client* target = server->get_client_by_nick(params[param_index]);
                    if (target && channel->is_client_in_channel(target)) {
                        channel->set_client_operator(target, adding);
                        changes += (adding ? "+" : "-");
                        changes += "o";
                        if (!change_params.empty()) {
                            change_params += " ";
                        }
                        change_params += params[param_index];
                    } else {
                        client.send_numeric_response(ERR_USERNOTINCHANNEL,
                                                     params[param_index] + " " + channel->get_name(),
                                                     "They aren't on that channel");
                    }
                    param_index++;
                }
                break;

            default:
                // Unknown mode - ignore for simplicity
                break;
        }
    }

    // Broadcast changes to all channel members
    if (!changes.empty()) {
        std::string mode_msg = client.get_prefix() + " MODE " + channel->get_name() + " " + changes;
        if (!change_params.empty()) {
            mode_msg += " " + change_params;
        }
        channel->broadcast(mode_msg);
    }
}

t_command_status Command::cmd_invite(Server* server) {
    if (parameters.size() < 2) {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    }

    std::string& target_nick = parameters[0];
    std::string& channel_name = parameters[1];

    // Check if channel exists
    if (!server->chan_man.channel_exists(channel_name)) {
        client.send_numeric_response(ERR_NOSUCHCHANNEL, channel_name, "No such channel");
        return CMD_FAILURE;
    }

    Channel* channel = server->chan_man.find_channel_by_name(channel_name);

    // Check if inviter is in the channel
    if (!channel->is_client_in_channel(&client)) {
        client.send_numeric_response(ERR_NOTONCHANNEL, channel_name, "You're not on that channel");
        return CMD_FAILURE;
    }

    // Check if inviter is operator (if channel is +i or +t)
    if (channel->is_invite_only() && !channel->is_client_operator(&client)) {
        client.send_numeric_response(ERR_CHANOPRIVSNEEDED, channel_name, "You're not channel operator");
        return CMD_FAILURE;
    }

    // Find target client
    Client* target_client = server->get_client_by_nick(target_nick);
    if (!target_client) {
        client.send_numeric_response(ERR_NOSUCHNICK, target_nick, "No such nick/channel");
        return CMD_FAILURE;
    }

    // Check if target is already in channel
    if (channel->is_client_in_channel(target_client)) {
        client.send_numeric_response(ERR_USERONCHANNEL, target_nick + " " + channel_name, "is already on channel");
        return CMD_FAILURE;
    }

    // Add to invite list
    channel->invite_client(target_client);

    // Send confirmation to inviter
    client.send_numeric_response(RPL_INVITING, target_nick + " " + channel_name, "");

    // Send invite message to target
    std::string invite_msg = client.get_prefix() + " INVITE " + target_nick + " " + channel_name;
    target_client->send_response(invite_msg);
    return CMD_SUCCESS;
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

    if (cmd_functions.find(this->command) != cmd_functions.end()) {
        log_msg(DEBUG, "Command found: " + this->command);
        return (this->*cmd_functions[this->command])(server);
    } else {
        return CMD_FAILURE;
    }
}
