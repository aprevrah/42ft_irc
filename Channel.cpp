#include "Channel.hpp"

Channel::Channel() : name(""), invite_only(false), topic_needs_op(false), user_limit(0) {
    (void) topic_needs_op;
}

Channel::Channel(const std::string &name) : name(name), invite_only(false), topic_needs_op(false), user_limit(0) {
}

Channel::~Channel() {
}

void Channel::join_client(Client* client, bool is_operator) {
    if (!client) {
        throw std::invalid_argument("client is null");
    }
    
    if (user_limit > 0 && clients.size() >= user_limit) {
        throw IRCException("Channel is full", ERR_CHANNELISFULL);
    }
    
    if (invite_only) {
        // TODO: implement invite system
    }
    
    // Add client to channel
    clients[client] = is_operator;
}

void Channel::leave_client(Client* client) {
    if (!client) {
        throw std::invalid_argument("client is null");
    }
    
    std::map<Client*, bool>::iterator it = clients.find(client);
    if (it != clients.end()) {
        clients.erase(it);
    } else 
        throw IRCException("Not on that channel", ERR_NOTONCHANNEL);
}

bool Channel::is_client_in_channel(Client* client) const {
    if (!client) {
        return false;
    }
    return clients.find(client) != clients.end();
}

bool Channel::is_client_operator(Client* client) const {
    if (!client) {
        return false;
    }
    std::map<Client *, bool>::const_iterator it = clients.find(client);
    if (it != clients.end()) {
        return it->second; // true if operator, false if regular user
    }
    return false;
}

const std::string& Channel::get_name() const {
    return name;
}

void Channel::set_topic(const std::string& new_topic) {
    topic = new_topic;
}

const std::string& Channel::get_topic() const {
    return topic;
}

size_t Channel::nbr_of_clients() const {
    return clients.size();
}

bool Channel::is_full() const {
    return clients.size() >= user_limit;
}