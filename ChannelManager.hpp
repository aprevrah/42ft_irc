#pragma once

#include <vector>
#include <string>
#include "Channel.hpp"

class Client;
class Channel;

class ChannelManager {
private:
    std::vector<Channel> channels;  //std::vector<Chan*> use heap?
    
    public:
    Channel* find_channel_by_name(const std::string& name);
    ChannelManager();
    ~ChannelManager();
    void join_channel(Client* client, const std::string& channel_name);
    void leave_channel(Client* client, const std::string& channel_name);
    bool channel_exists(const std::string& name);

    /**
     * @brief This is for when a client quits/disconnets from the server
     */
    void quit_all_channels(Client &client, std::string reaso);
};