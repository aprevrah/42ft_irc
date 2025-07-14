#pragma once
#include <string>
#include <vector>
#include <ostream>

class Command {
   private:

   public:
    std::string              prefix;
    std::string              command;
    std::vector<std::string> parameters;
    Command();
    Command(std::string command_str);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();
    
    friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
};


