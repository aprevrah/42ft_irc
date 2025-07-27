#include "Command.hpp"

t_command_status Command::cmd_nick(Server* server) {
    (void)server;
    if (parameters.size() > 0) {
        if (server->is_nick_available(parameters.front())) {
            client.set_nickname(parameters.front());
        } else {
            client.send_numeric_response(ERR_NICKNAMEINUSE, parameters.front(), "Nickname is already in use");
            return CMD_FAILURE;
        }

        // std::string response = "TestResponse: Nick set to " + client.get_nickname();
        // client.send_response(response); //TODO: Check if nick is valid and error responses
        client.try_register();
        return CMD_SUCCESS;
    } else {
        client.send_numeric_response(ERR_NONICKNAMEGIVEN, std::string(), "No nickname given");
        return CMD_FAILURE;
    }
}
