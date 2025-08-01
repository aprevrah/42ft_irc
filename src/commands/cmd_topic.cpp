#include "Command.hpp"

t_command_status Command::cmd_topic(Server* server) {
    // TOPIC command requires at least a channel name
    if (parameters.empty()) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "TOPIC", "Not enough parameters");
        return CMD_SUCCESS;
    }

    std::string channel_name = parameters.at(0);
    Channel* channel = server->chan_man.find_channel_by_name(channel_name);

    if (!channel) {
        client.send_numeric_response(ERR_NOSUCHCHANNEL, channel_name, "No such channel");
        return CMD_SUCCESS;
    }

    if (!channel->is_client_in_channel(&client)) {
        client.send_numeric_response(ERR_NOTONCHANNEL, channel_name, "You're not on that channel");
        return CMD_SUCCESS;
    }

    if (parameters.size() == 1) {
        std::string current_topic = channel->get_topic();
        if (current_topic.empty()) {
            client.send_numeric_response(RPL_NOTOPIC, channel_name, "No topic is set");
        } else {
            client.send_numeric_response(RPL_TOPIC, channel_name, current_topic);
        }
        return CMD_SUCCESS;
    }

    std::string new_topic = parameters.at(1);

    if (channel->is_topic_restricted() && !channel->is_client_operator(&client)) {
        client.send_numeric_response(ERR_CHANOPRIVSNEEDED, channel_name, "You're not channel operator");
        return CMD_SUCCESS;
    }

    channel->set_topic(new_topic);

    std::string topic_message = client.get_prefix() + " TOPIC " + channel_name + " :" + new_topic;
    channel->broadcast(topic_message);

    return CMD_SUCCESS;
}
