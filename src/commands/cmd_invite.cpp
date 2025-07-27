#include "Command.hpp"

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
