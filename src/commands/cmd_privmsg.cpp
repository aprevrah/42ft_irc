#include "Command.hpp"

t_command_status Command::cmd_privmsg(Server* server) {
    if (parameters.size() < 2) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "PRIVMSG", "Not enough parameters");
        return CMD_FAILURE;
    }
    std::vector<std::string> targets = split_string(parameters[0], ',');
    std::string&             message = parameters[1];

    for (size_t i = 0; i < targets.size(); i++) {
        std::string& target = targets[i];
        if (target.empty()) {
            continue;
        }
        if (target[0] == '#' || target[0] == '&') {
            Channel* channel = server->chan_man.find_channel_by_name(target);
            if (!channel) {
                client.send_numeric_response(ERR_NOSUCHCHANNEL, target, "No such channel");
                continue;
            }
            if (!channel->is_client_in_channel(&client)) {
                client.send_numeric_response(ERR_CANNOTSENDTOCHAN, target, "Cannot send to channel");
                continue;
            }

            std::string privmsg = client.get_prefix() + " PRIVMSG " + target + " :" + message;
            channel->broadcast(privmsg, &client);
        } else {
            Client* target_client = server->get_client_by_nick(target);
            if (!target_client) {
                client.send_numeric_response(ERR_NOSUCHNICK, target, "No such nick/channel");
                continue;
            }
            // Build the private message
            std::string privmsg = client.get_prefix() + " PRIVMSG " + target + " :" + message;
            target_client->send_response(privmsg);
        }
    }
    return CMD_SUCCESS;
}
