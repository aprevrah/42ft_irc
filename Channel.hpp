#pragma once
#include <map>
#include <set>
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
    
    // Invite system
    std::set<Client*> invited_clients;

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
    void               broadcast(const std::string &msg) const;
    
    // Mode methods
    bool is_invite_only() const;
    bool is_topic_restricted() const;
    bool has_key() const;
    bool has_user_limit() const;
    unsigned int get_user_limit() const;
    const std::string& get_key() const;
    
    void set_invite_only(bool value);
    void set_topic_restricted(bool value);
    void set_key(const std::string& key);
    void set_user_limit(unsigned int limit);
    void set_client_operator(Client* client, bool is_op);
    
    // Invite system
    void invite_client(Client* client);
    bool is_client_invited(Client* client) const;
    void remove_invite(Client* client);
    
    // For ChannelManager access
    const std::map<Client*, bool>& get_clients() const;
};