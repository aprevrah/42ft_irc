#include "Command.hpp"

t_command_status Command::cmd_part(Server* server) {
    if (parameters.size() == 0) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "PART", "Not enough parameters");
        return CMD_FAILURE;
    }
    
    std::vector<std::string> channels = split_string(parameters[0], ',');
    std::string reason = "";
    if (parameters.size() > 1) {
        reason = parameters[1];
    }

    for (size_t i = 0; i < channels.size(); i++) {
        part_chan(server, channels[i], reason);
    }
    return CMD_SUCCESS;
}

int Command::part_chan(Server* server, const std::string& chan_name, const std::string& reason) {
    int error_code = server->chan_man.leave_channel(client, chan_name);
    
    if (error_code != 0) {
        // Handle error cases
        if (error_code == ERR_NOSUCHCHANNEL) {
            client.send_numeric_response(ERR_NOSUCHCHANNEL, chan_name, "No such channel");
        } else if (error_code == ERR_NOTONCHANNEL) {
            client.send_numeric_response(ERR_NOTONCHANNEL, chan_name, "You're not on that channel");
        }
        return 1;
    }
    
    // Success - send part message
    std::string part_msg = client.get_prefix() + " PART " + chan_name;
    if (!reason.empty()) {
        part_msg += " :" + reason;
    }
    Channel* channel = server->chan_man.find_channel_by_name(chan_name);
    if (channel) {
        channel->broadcast(part_msg, NULL);
    }
    client.send_response(part_msg);
    return 0;
}
