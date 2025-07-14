#pragma once
#include <string>
#include <vector>
#include <ostream>

class Command {
   private:
    std::string              command;
    std::string              prefix;
    std::vector<std::string> parameters;

   public:
    Command();
    Command(std::string command_str);
    Command(const Command& other);
    Command& operator=(const Command& other);
    ~Command();
    
    friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
};


