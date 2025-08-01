#include "Channel.hpp"

#include "Client.hpp"

Channel::Channel() : name(""), invite_only(false), topic_needs_op(false), user_limit(0) {
    (void)topic_needs_op;
}

Channel::Channel(const std::string& name) : name(name), invite_only(false), topic_needs_op(false), user_limit(0) {}

Channel::~Channel() {}

int Channel::join_client(Client* client, bool is_operator) {
    if (user_limit > 0 && clients.size() >= user_limit) {
        return ERR_CHANNELISFULL;
    }

    if (invite_only) {
        // Check if client is invited
        if (!is_client_invited(client)) {
            return ERR_INVITEONLYCHAN;
        }
        // Remove from invite list once they join
        remove_invite(client);
    }

    // Add client to channel
    clients[client] = is_operator;
    return 0;
}

int Channel::leave_client(Client* client) {
    std::map<Client*, bool>::iterator it = clients.find(client);
    if (it != clients.end()) {
        clients.erase(it);
    } else {
        return ERR_NOTONCHANNEL;
    }
    return 0;
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
    std::map<Client*, bool>::const_iterator it = clients.find(client);
    if (it != clients.end()) {
        return it->second;  // true if operator, false if regular user
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

void Channel::broadcast(const std::string& msg, Client* sender) const {
    for (std::map<Client*, bool>::const_iterator it = clients.begin(); it != clients.end(); it++) {
        Client* client = it->first;
        if (client && client != sender) {
            client->send_response(msg);
        }
    }
}

void Channel::broadcast(const std::string& msg) const {
    for (std::map<Client*, bool>::const_iterator it = clients.begin(); it != clients.end(); it++) {
        Client* client = it->first;
        if (client) {
            client->send_response(msg);
        }
    }
}

// Mode methods
bool Channel::is_invite_only() const {
    return invite_only;
}

bool Channel::is_topic_restricted() const {
    return topic_needs_op;
}

bool Channel::has_key() const {
    return !passwd.empty();
}

bool Channel::has_user_limit() const {
    return user_limit > 0;
}

unsigned int Channel::get_user_limit() const {
    return user_limit;
}

const std::string& Channel::get_key() const {
    return passwd;
}

void Channel::set_invite_only(bool value) {
    invite_only = value;
}

void Channel::set_topic_restricted(bool value) {
    topic_needs_op = value;
}

void Channel::set_key(const std::string& key) {
    passwd = key;
}

void Channel::set_user_limit(unsigned int limit) {
    user_limit = limit;
}

void Channel::set_client_operator(Client* client, bool is_op) {
    if (client && is_client_in_channel(client)) {
        clients.at(client) = is_op;
    }
}

// Invite system methods
void Channel::invite_client(Client* client) {
    if (client) {
        invited_clients.insert(client);
    }
}

bool Channel::is_client_invited(Client* client) const {
    if (!client) {
        return false;
    }
    return invited_clients.find(client) != invited_clients.end();
}

void Channel::remove_invite(Client* client) {
    if (client) {
        invited_clients.erase(client);
    }
}

const std::map<Client*, bool>& Channel::get_clients() const {
    return clients;
}

void Channel::send_topic_to_client(Client& client) const {
    if (!topic.empty()) {
        client.send_numeric_response(RPL_TOPIC, name, topic);
    } else {
        client.send_numeric_response(RPL_NOTOPIC, name, "No topic is set");
    }
}

void Channel::send_names_to_client(Client& client) const {
    std::string names_list = "";

    for (std::map<Client*, bool>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it != clients.begin()) {
            names_list += " ";
        }
        if (it->second) {  // if client is operator
            names_list += "@";
        }
        names_list += it->first->get_nickname();
    }

    client.send_numeric_response(RPL_NAMREPLY, "= " + name, names_list);
    client.send_numeric_response(RPL_ENDOFNAMES, name, "End of NAMES list");
}

int Channel::kick_client(Client* target, Client* kicker, const std::string& comment) {
    if (!target) {
        return ERR_NOSUCHNICK;
    }

    if (!kicker) {
        return ERR_NOSUCHNICK;  // or could be a different error for invalid kicker
    }

    // Check if kicker is in the channel
    if (!is_client_in_channel(kicker)) {
        return ERR_NOTONCHANNEL;
    }

    // Check if kicker has operator privileges
    if (!is_client_operator(kicker)) {
        return ERR_CHANOPRIVSNEEDED;
    }

    // Check if target is in the channel
    if (!is_client_in_channel(target)) {
        return ERR_USERNOTINCHANNEL;
    }

    // Construct KICK message
    std::string kick_msg = kicker->get_prefix() + " KICK " + name + " " + target->get_nickname() + " :" + comment;

    // Broadcast the KICK message to all channel members
    broadcast(kick_msg);

    // Remove the target client from the channel
    int error_code = leave_client(target);
    if (error_code != 0) {
        return error_code;  // Propagate any error from leave_client
    }
    
    return 0;  // Success
}