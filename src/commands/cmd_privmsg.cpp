#include "Command.hpp"

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
