#include "Command.hpp"

t_command_status Command::cmd_join(Server* server) {  // TODO: join multiple channels
    if (parameters.size() > 0) {
        std::string& chan_name = parameters.front();
        std::string  key = "";

        // Check if key is provided: JOIN #channel key
        if (parameters.size() > 1) {
            key = parameters[1];
        }

        try {
            server->chan_man.join_channel(&client, chan_name, key);

            std::string join_msg = client.get_prefix() + " JOIN " + chan_name;
            Channel*    channel = server->chan_man.find_channel_by_name(chan_name);
            if (channel) {
                channel->broadcast(join_msg, NULL);
            }
            return CMD_SUCCESS;
        } catch (IRCException& e) {
            client.send_numeric_response(e.get_irc_numeric(), chan_name, e.what());
            return CMD_FAILURE;
        } catch (std::exception& e) {
            client.send_response(std::string("JOIN fail: ") + e.what());
            return CMD_FAILURE;
        }
    }
    return CMD_FAILURE;
}
