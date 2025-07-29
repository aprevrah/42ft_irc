#include "Command.hpp"

t_command_status Command::cmd_join(Server* server) {  // TODO: join multiple channels
    if (parameters.size() == 0) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "JOIN", "Not enough parameters");
        return CMD_FAILURE;
    }

    std::string& chan_name = parameters.front();
    std::string  key = "";

    // Check if key is provided: JOIN #channel key
    if (parameters.size() > 1) {
        key = parameters[1];
    }

    int error_code = server->chan_man.join_channel(client, chan_name, key);
    if (error_code != 0) {
        if (error_code == ERR_BADCHANMASK) {
            client.send_numeric_response(ERR_BADCHANMASK, chan_name, "Bad Channel Mask");
        } else if (error_code == ERR_BADCHANNELKEY) {
            client.send_numeric_response(ERR_BADCHANNELKEY, chan_name, "Cannot join channel (+k)");
        } else if (error_code == ERR_CHANNELISFULL) {
            client.send_numeric_response(ERR_CHANNELISFULL, chan_name, "Cannot join channel (+l)");
        } else if (error_code == ERR_INVITEONLYCHAN) {
            client.send_numeric_response(ERR_INVITEONLYCHAN, chan_name, "Cannot join channel (+i)");
        }
        return CMD_FAILURE;
    }

    // Success - send join message and channel info
    std::string join_msg = client.get_prefix() + " JOIN " + chan_name;
    Channel*    channel = server->chan_man.find_channel_by_name(chan_name);
    if (channel) {
        channel->broadcast(join_msg, NULL);
        // Send channel info to the joining user
        channel->send_topic_to_client(client);
        channel->send_names_to_client(client);
    }
    return CMD_SUCCESS;
}
