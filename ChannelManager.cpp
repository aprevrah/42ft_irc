#include "ChannelManager.hpp"
#include "Client.hpp"

ChannelManager::ChannelManager() {}

ChannelManager::~ChannelManager() {}

Chan* ChannelManager::find_channel_by_name(const std::string& name) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].get_name() == name) {
            return &channels[i];
        }
    }
    return NULL;
}
