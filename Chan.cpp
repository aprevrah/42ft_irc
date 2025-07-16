#include "Chan.hpp"

Chan::Chan() : name(""), invite_only(false), topic_needs_op(false), user_limit(0) {
}

Chan::Chan(const std::string &name) : name(name), invite_only(false), topic_needs_op(false), user_limit(0) {
}

Chan::~Chan() {
}

bool Chan::join_client(Client* client, bool is_operator) {
    if (!client) {
        return false;
    }
    
    // Check if channel has user limit
    if (user_limit > 0 && clients.size() >= user_limit) {
        return false;
    }
    
    // Check if invite only and client is not invited (simplified for now)
    if (invite_only) {
        // TODO: implement invite system
    }
    
    // Add client to channel
    clients[client] = is_operator;
    return true;
}

bool Chan::leave_client(Client* client) {
    if (!client) {
        return false;
    }
    
    std::map<Client*, bool>::iterator it = clients.find(client);
    if (it != clients.end()) {
        clients.erase(it);
        return true;
    }
    return false;
}

bool Chan::is_client_in_channel(Client* client) const {
    if (!client) {
        return false;
    }
    return clients.find(client) != clients.end();
}

bool Chan::is_client_operator(Client* client) const {
    if (!client) {
        return false;
    }
    std::map<Client *, bool>::const_iterator it = clients.find(client);
    if (it != clients.end()) {
        return it->second; // true if operator, false if regular user
    }
    return false;
}

const std::string& Chan::get_name() const {
    return name;
}

void Chan::set_topic(const std::string& new_topic) {
    topic = new_topic;
}

const std::string& Chan::get_topic() const {
    return topic;
}
