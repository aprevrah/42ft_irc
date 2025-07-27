#include "Command.hpp"

t_command_status Command::cmd_quit(Server* server) {
    std::string reason = parameters.size() > 0 ? parameters.back() : "quit";
    server->disconnect_client(client.get_fd(), reason);
    return CLIENT_DISCONNECTED;
}
