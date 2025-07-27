#pragma once
#include <exception>
#include <string>

class IRCException : public std::exception {
   private:
    std::string  message;
    unsigned int numeric;

   public:
    IRCException(const std::string& msg, unsigned int irc_numeric) : message(msg), numeric(irc_numeric) {}

    virtual ~IRCException() throw() {}
    const char* what() const throw() {
        return message.c_str();
    }
    unsigned int get_irc_numeric() const {
        return numeric;
    }
};