#include "Command.hpp"

t_command_status Command::cmd_pass(Server* server) {
    if (parameters.size() < 1) {
        log_msg(WARNING, "PASS command: Not enough parameters");
        client.send_numeric_response(ERR_NEEDMOREPARAMS, "PASS", "Not enough parameters");
        return CMD_FAILURE;
    }
    if (client.is_registered()) {
        client.send_numeric_response(ERR_ALREADYREGISTERED, "PASS", "You may not reregister");
        return CMD_FAILURE;
    }
    // check if already registered --> ERR_ALREADYREGISTRED
    client.correct_password = server->is_correct_password(parameters.at(0));
    log_msg(DEBUG, "client.correct_password: " + to_string(client.correct_password));
    return CMD_SUCCESS;
}
