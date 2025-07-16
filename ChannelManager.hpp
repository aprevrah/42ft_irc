#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include <vector>
#include <string>
#include "Chan.hpp"

class Client;

class ChannelManager {
private:
    std::vector<Chan> channels;  //std::vector<Chan*> use heap?
    Chan* find_channel_by_name(const std::string& name);
    
public:
    ChannelManager();
    ~ChannelManager();
    bool join_channel(Client* client, const std::string& channel_name);
    bool leave_channel(Client* client, const std::string& channel_name);
    bool channel_exists(const std::string& name);
};