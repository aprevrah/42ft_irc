#include "Command.hpp"

t_command_status Command::cmd_ping(Server* server) {
    (void)server;
    if (parameters.size() > 0) {
        std::string& token = parameters.back();
        client.send_response("PONG " SERVER_NAME " " + token);  // TODO: server name var?
        return CMD_SUCCESS;
    } else {
        client.send_response(to_string(ERR_NEEDMOREPARAMS));
        return CMD_FAILURE;
    }
}
