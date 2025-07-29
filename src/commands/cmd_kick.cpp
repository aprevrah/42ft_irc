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
    if (parameters.size() > 2 && !parameters[2].empty()) {
        comment = parameters[2];
    } else {
        comment = "Kicked by " + client.get_nickname();
    }

    // Find the channel
    Channel* channel = server->chan_man.find_channel_by_name(channel_name);
    if (!channel) {
        client.send_numeric_response(ERR_NOSUCHCHANNEL, channel_name, "No such channel");
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

            // Use the channel's kick method
            int error_code = channel->kick_client(target_client, &client, comment);
            if (error_code != 0) {
                // Handle specific error cases
                if (error_code == ERR_NOSUCHNICK) {
                    client.send_numeric_response(ERR_NOSUCHNICK, target_nick, "No such nick/channel");
                } else if (error_code == ERR_NOTONCHANNEL) {
                    client.send_numeric_response(ERR_NOTONCHANNEL, channel_name, "You're not on that channel");
                } else if (error_code == ERR_CHANOPRIVSNEEDED) {
                    client.send_numeric_response(ERR_CHANOPRIVSNEEDED, channel_name, "You're not channel operator");
                } else if (error_code == ERR_USERNOTINCHANNEL) {
                    client.send_numeric_response(ERR_USERNOTINCHANNEL, target_nick + " " + channel_name, "They aren't on that channel");
                }
                continue;  // Continue with next user
            }
        }

        return CMD_SUCCESS;
}
