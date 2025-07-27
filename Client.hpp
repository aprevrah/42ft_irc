#pragma once
#include <string.h>

#include <cstring>
#include <string>

#include "Command.hpp"
#include "ft_irc.hpp"
#include "to_string.hpp"

class Server;  // Forward declaration

#define MESSAGE_BUFFER_SIZE 512

class Client {
   private:
    Server*     server;
    int         fd;
    std::string message_buffer;
    std::string nickname;
    std::string username;
    std::string hostname;
    bool        registered;

   public:
    bool correct_password;
    Client();
    Client(int fd, Server* server);
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();

    t_command_status add_to_buffer(std::string new_bytes);
    int              get_fd() const;

    void send_response(const std::string& response);

    /**
     * @brief Sends a numeric reply to the client.
     *
     * The client's nickname is automatically added as the first parameter.
     *
     * @param numeric The 3-digit numeric reply code.
     * @param params Space-separated parameters for the reply.
     * @param message The trailing part of the message, which will be prefixed with a colon.
     */
    void send_numeric_response(const unsigned int numeric, std::string params, const std::string& message);

    const std::string& get_nickname() const;
    const std::string& get_hostname() const;
    void               set_nickname(const std::string& nickname);
    bool               try_register();
    bool               is_registered();
    void               set_username(std::string);
    std::string        get_prefix() const;
};
