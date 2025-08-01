#pragma once

#include <string>
#include <vector>

#include "Channel.hpp"

class Client;
class Channel;

class ChannelManager {
   private:
    std::vector<Channel> channels;  // std::vector<Chan*> use heap?

   public:
    Channel* find_channel_by_name(const std::string& name);
    ChannelManager();
    ~ChannelManager();
    int join_channel(Client& client, const std::string& channel_name, const std::string& key = "");
    int leave_channel(Client& client, const std::string& channel_name);
    bool channel_exists(const std::string& name);
    bool is_valid_channel_name(const std::string& name);
    std::vector<std::string> get_client_channels(Client& client);

    /**
     * @brief This is for when a client quits/disconnets from the server
     */
    void quit_all_channels(Client& client, std::string reaso);
    void broadcast_shared_channels(Client& client, const std::string& msg);
};