#include "Command.hpp"

t_command_status Command::cmd_join(Server* server) {  // TODO: join multiple channels
    if (parameters.size() < 1) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "JOIN", "Not enough parameters");
        return CMD_FAILURE;
    }
    if (parameters[0].empty()) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "JOIN", "Not enough parameters");
        return CMD_FAILURE;
    }
    
    std::vector<std::string> channels = split_string(parameters[0], ',');
    std::vector<std::string> keys;
    
    if (parameters.size() > 1) {
        keys = split_string(parameters[1], ',');
    }

    for (size_t i = 0; i < channels.size(); i++) {
        std::string key = "";
        if (i < keys.size())
            key = keys[i];
        
        join_chan(server, channels[i], key);
    }

    return CMD_SUCCESS;
}

int Command::join_chan(Server* server, const std::string& chan_name, const std::string& key) {
    
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
        return 1;
    }

    std::string join_msg = client.get_prefix() + " JOIN " + chan_name;
    Channel*    channel = server->chan_man.find_channel_by_name(chan_name);
    if (channel) {
        channel->broadcast(join_msg, NULL);
        //channel info
        channel->send_topic_to_client(client);
        channel->send_names_to_client(client);
    }
    return 0;
}