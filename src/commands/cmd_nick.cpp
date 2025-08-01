#include "Command.hpp"
bool is_valid_nickname(const std::string& nick) {
    // RFC 2812: Nickname must not be empty and max 9 characters
    if (nick.empty() || nick.length() > 9) {
        return false;
    }
    
    const std::string forbidden = " ,:\t\r\n";
    const std::string special = "[]\\`_^{|}";
    
    for (size_t i = 0; i < nick.length(); i++) {
        char c = nick.at(i);
        
        if (forbidden.find(c) != std::string::npos) {
            return false;
        }
        
        if (i == 0) {
            if (!std::isalpha(c) && special.find(c) == std::string::npos) {
                return false;
            }
        } else {
            if (!std::isalnum(c) && special.find(c) == std::string::npos && c != '-') {
                return false;
            }
        }
    }
    return true;
}

t_command_status Command::cmd_nick(Server* server) {
    if (parameters.size() == 0) {
        client.send_numeric_response(ERR_NONICKNAMEGIVEN, std::string(), "No nickname given");
        return CMD_FAILURE;
    }

    std::string new_nick = parameters.front();

    if (!is_valid_nickname(new_nick)) {
        client.send_numeric_response(ERR_ERRONEUSNICKNAME, new_nick, "Erroneous nickname");
        return CMD_FAILURE;
    }

    if (!server->is_nick_available(new_nick)) {
        client.send_numeric_response(ERR_NICKNAMEINUSE, new_nick, "Nickname is already in use");
        return CMD_FAILURE;
    }

    std::string old_nick = client.get_nickname();
    
    if (!old_nick.empty() && client.is_registered()) {
        std::string nick_msg = client.get_prefix() + " NICK " + new_nick;
        
        client.send_response(nick_msg);
        server->chan_man.broadcast_shared_channels(client, nick_msg);
    }
    client.set_nickname(new_nick);
    
    client.try_register();
    
    return CMD_SUCCESS;
    
}
