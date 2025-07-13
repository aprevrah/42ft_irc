#pragma once
#include <string>
#include <vector>

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
};
