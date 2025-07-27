#include "Command.hpp"

// Parameters: <username> <hostname> <servername> <realname>
t_command_status Command::cmd_user(Server* server) {
    (void)server;
    if (parameters.size() < 4) {
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "USER", "Not enough parameters");
        return CMD_FAILURE;
    }
    if (client.is_registered()) {
        client.send_numeric_response(ERR_ALREADYREGISTERED, "USER", "You may not reregister");
    }
    client.set_username(parameters.at(0));
    // TODO: do sth with the other parameters?
    client.try_register();
    return CMD_SUCCESS;
}
