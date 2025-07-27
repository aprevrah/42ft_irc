#include "Command.hpp"

t_command_status Command::cmd_cap(Server* server) {
    (void)server;
    if (parameters.size() > 0 && parameters.front() == "LS") {
        client.send_response("CAP * LS :");
    }
    return CMD_SUCCESS;
}
