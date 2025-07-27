#include "Command.hpp"

t_command_status Command::cmd_kick(Server* server) {
    // KICK <channel> <user> *( "," <user> ) [<comment>]

    // Check if we have enough parameters
    if (parameters.size() < 2) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters");
        return CMD_FAILURE;
    }

    std::string channel_name = parameters[0];
    std::string users_param = parameters[1];
    std::string comment = "";

    // Extract comment if provided
    if (parameters.size() > 2) {
        comment = parameters[2];
    } else {
        comment = "Kicked by " + client.get_nickname();
    }

    try {
        // Find the channel
        Channel* channel = server->chan_man.find_channel_by_name(channel_name);
        if (!channel) {
            client.send_numeric_response(ERR_NOSUCHCHANNEL, channel_name, "No such channel");
            return CMD_FAILURE;
        }

        // Check if the client is in the channel
        if (!channel->is_client_in_channel(&client)) {
            client.send_numeric_response(ERR_NOTONCHANNEL, channel_name, "You're not on that channel");
            return CMD_FAILURE;
        }

        // Check if the client has operator privileges
        if (!channel->is_client_operator(&client)) {
            client.send_numeric_response(ERR_CHANOPRIVSNEEDED, channel_name, "You're not channel operator");
            return CMD_FAILURE;
        }

        // Parse multiple users (comma-separated)
        std::vector<std::string> target_nicks;
        std::string              current_nick = "";
        for (size_t i = 0; i < users_param.length(); ++i) {
            if (users_param[i] == ',') {
                if (!current_nick.empty()) {
                    target_nicks.push_back(current_nick);
                    current_nick = "";
                }
            } else {
                current_nick += users_param[i];
            }
        }
        if (!current_nick.empty()) {
            target_nicks.push_back(current_nick);
        }

        // Process each target nick individually
        for (std::vector<std::string>::iterator it = target_nicks.begin(); it != target_nicks.end(); ++it) {
            std::string target_nick = *it;

            // Find the target client by nickname
            Client* target_client = server->get_client_by_nick(target_nick);
            if (!target_client) {
                client.send_numeric_response(ERR_NOSUCHNICK, target_nick, "No such nick/channel");
                continue;  // Continue with next user instead of returning failure
            }

            // Check if the target client is in the channel
            if (!channel->is_client_in_channel(target_client)) {
                client.send_numeric_response(ERR_USERNOTINCHANNEL, target_nick + " " + channel_name,
                                             "They aren't on that channel");
                continue;  // Continue with next user instead of returning failure
            }

            // Construct KICK message
            std::string kick_msg = client.get_prefix() + " KICK " + channel_name + " " + target_nick + " :" + comment;

            // Broadcast the KICK message to all channel members
            channel->broadcast(kick_msg, NULL);

            // Remove the target client from the channel
            channel->leave_client(target_client);
        }

        return CMD_SUCCESS;

    } catch (IRCException& e) {
        client.send_numeric_response(e.get_irc_numeric(), channel_name, e.what());
        return CMD_FAILURE;
    } catch (std::exception& e) {
        client.send_response(std::string("KICK fail: ") + e.what());
        return CMD_FAILURE;
    }
}
