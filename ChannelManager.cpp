#include "ChannelManager.hpp"
#include "Client.hpp"

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

void ChannelManager::join_channel(Client* client, const std::string& channel_name) {
    Channel* channel = find_channel_by_name(channel_name);
    if (channel) {
        return channel->join_client(client);
    } else {
        Channel new_channel(channel_name);
        channels.push_back(new_channel);
        return channels.back().join_client(client, true); //first user op
    }
}

void ChannelManager::leave_channel(Client* client, const std::string& channel_name) {
    Channel* channel = find_channel_by_name(channel_name);
    
    if (channel) {
        channel->leave_client(client);
        // Optionally remove empty channels here
    } else {
        throw   IRCException("Not such chan", ERR_NOSUCHCHANNEL);
    }
}