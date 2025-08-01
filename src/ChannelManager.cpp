#include "ChannelManager.hpp"

#include "Client.hpp"
#include "Numerics.hpp"

ChannelManager::ChannelManager() {}

ChannelManager::~ChannelManager() {}

Channel* ChannelManager::find_channel_by_name(const std::string& name) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].get_name() == name) {
            return &channels[i];
        }
    }
    return NULL;
}

bool ChannelManager::channel_exists(const std::string& name) {
    return find_channel_by_name(name) != NULL;
}

int ChannelManager::join_channel(Client& client, const std::string& channel_name, const std::string& key) {
    if (!is_valid_channel_name(channel_name)) {
        return ERR_BADCHANMASK;
    }
    Channel* channel = find_channel_by_name(channel_name);
    if (channel) {
        // Check if channel has a key and key is required
        if (channel->has_key() && channel->get_key() != key) {
            return ERR_BADCHANNELKEY;
        }
        return channel->join_client(&client);
    } else {
        Channel new_channel(channel_name);
        // Set the key if provided during channel creation
        if (!key.empty()) {
            new_channel.set_key(key);
        }
        channels.push_back(new_channel);
        return channels.back().join_client(&client, true);  // first user op
    }
}

int ChannelManager::leave_channel(Client& client, const std::string& channel_name) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].get_name() == channel_name) {
            int error_code = channels[i].leave_client(&client);
            if (error_code != 0) {
                return error_code;  // Propagate error from Channel::leave_client
            }
            if (channels[i].get_clients().empty()) {
                channels.erase(channels.begin() + i);
            }
            return 0;
        }
    }
    return ERR_NOSUCHCHANNEL;
}

void ChannelManager::broadcast_shared_channels(Client& client, const std::string& msg) {
    std::set<Client*> clients_to_notify;
    
    // Collect all unique clients who share channels with the given client
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].is_client_in_channel(&client)) {
            const std::map<Client*, bool>& channel_clients = channels[i].get_clients();
            for (std::map<Client*, bool>::const_iterator it = channel_clients.begin(); 
                 it != channel_clients.end(); ++it) {
                if (it->first && it->first != &client) {
                    clients_to_notify.insert(it->first);
                }
            }
        }
    }
    
    // Send message once to each unique client
    for (std::set<Client*>::iterator it = clients_to_notify.begin(); 
         it != clients_to_notify.end(); ++it) {
        (*it)->send_response(msg);
    }
}

std::vector<std::string> ChannelManager::get_client_channels(Client& client) {
    std::vector<std::string> channel_names;
    
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].is_client_in_channel(&client)) {
            channel_names.push_back(channels[i].get_name());
        }
    }
    
    return channel_names;
}

void ChannelManager::quit_all_channels(Client& client, std::string reason) {
    std::string quit_msg = client.get_prefix() + " QUIT :" + reason;
    
    // Use existing broadcast method to notify all shared channel clients
    broadcast_shared_channels(client, quit_msg);
    
    // Remove client from all channels
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].is_client_in_channel(&client)) {
            leave_channel(client, channels[i].get_name());
        }
    }
}

bool ChannelManager::is_valid_channel_name(const std::string& name) {
    // Channel name must not be empty
    if (name.empty()) {
        return false;
    }
    
    // Channel name must start with # or &
    if (name[0] != '#' && name[0] != '&') {
        return false;
    }
    
    // Channel name must be at least 2 characters (prefix + name)
    if (name.length() < 2) {
        return false;
    }
    
    // Channel names cannot contain spaces, commas, or control characters
    for (size_t i = 0; i < name.length(); i++) {
        char c = name[i];
        if (c == ' ' || c == ',' || c == '\0' || c == '\r' || c == '\n' || c == 7) {
            return false;
        }
    }
    
    return true;
}