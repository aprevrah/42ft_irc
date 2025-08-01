#include "Command.hpp"

t_command_status Command::cmd_ping(Server* server) {
    (void)server;
    if (parameters.size() > 0) {
        std::string& token = parameters.back();
        client.send_response("PONG " SERVER_NAME " :" + token);
        return CMD_SUCCESS;
    } else {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "PING", "Not enough parameters");
        return CMD_FAILURE;
    }
}
