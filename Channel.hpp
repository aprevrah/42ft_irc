#pragma once
#include <map>
#include <stdexcept>
#include <string>

#include "IRCException.hpp"
#include "Numerics.hpp"


class Client;

class Channel {
   private:
    // bool is true if the client is operator
    std::map<Client*, bool> clients;
    std::string             name;
    std::string             topic;
    // modes
    bool         invite_only;
    bool         topic_needs_op;
    std::string  passwd;
    unsigned int user_limit;  // 0 is no limit

   public:
    Channel();
    Channel(const std::string& name);
    ~Channel();

    // Channel operations
    bool               is_full() const;
    size_t             nbr_of_clients() const;
    void               join_client(Client* client, bool is_operator = false);
    void               leave_client(Client* client);
    bool               is_client_in_channel(Client* client) const;
    bool               is_client_operator(Client* client) const;
    const std::string& get_name() const;
    void               set_topic(const std::string& new_topic);
    const std::string& get_topic() const;
    void               broadcast(const std::string &msg, Client *sender) const;
};