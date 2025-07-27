#include "Command.hpp"

t_command_status Command::cmd_part(Server* server) {
    if (parameters.size() == 0) {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    } else {
        std::string& chan_name = parameters.front();
        try {
            Channel*    channel = server->chan_man.find_channel_by_name(chan_name);
            std::string part_msg = client.get_prefix() + " PART " + chan_name;
            server->chan_man.leave_channel(&client, chan_name);
            if (channel) {
                channel->broadcast(part_msg, NULL);
                client.send_response(part_msg);
            }
            return CMD_SUCCESS;
        } catch (IRCException& e) {
            client.send_response(to_string(e.get_irc_numeric()) + std::string(" ") + e.what());  // TODO: correct msg
            return CMD_FAILURE;
        } catch (std::exception& e) {
            client.send_response(std::string("PART fail: ") + e.what());
            return CMD_FAILURE;
        }
    }
}
