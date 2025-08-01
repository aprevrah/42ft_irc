#include "Command.hpp"

t_command_status Command::cmd_mode(Server* server) {
    if (parameters.size() < 1) {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    }

    std::string& target = parameters.at(0);

    // Only handle channel modes (starting with # or &)
    if (target.at(0) != '#' && target.at(0) != '&') {
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
    std::string&             modes = parameters.at(1);
    std::vector<std::string> mode_params;

    // Collect mode parameters
    for (size_t i = 2; i < parameters.size(); i++) {
        mode_params.push_back(parameters.at(i));
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
        char mode = modes.at(i);

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
                        channel->set_key(params.at(param_index));
                        changes += "+k";
                        if (!change_params.empty()) {
                            change_params += " ";
                        }
                        change_params += params.at(param_index);
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
                        int limit = atoi(params.at(param_index).c_str());
                        if (limit > 0) {
                            channel->set_user_limit(limit);
                            changes += "+l";
                            if (!change_params.empty()) {
                                change_params += " ";
                            }
                            change_params += params.at(param_index);
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
                    Client* target = server->get_client_by_nick(params.at(param_index));
                    if (target && channel->is_client_in_channel(target)) {
                        channel->set_client_operator(target, adding);
                        changes += (adding ? "+" : "-");
                        changes += "o";
                        if (!change_params.empty()) {
                            change_params += " ";
                        }
                        change_params += params.at(param_index);
                    } else {
                        client.send_numeric_response(ERR_USERNOTINCHANNEL,
                                                     params.at(param_index) + " " + channel->get_name(),
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
