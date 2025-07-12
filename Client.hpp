#pragma once
#include <string>
#define MESSAGE_BUFFER_SIZE 512

class Client {
   private:
    int         fd;
    char        message_buffer[MESSAGE_BUFFER_SIZE + 1];
    std::string nickname;

   public:
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();
    void add_to_buffer(char* new_bytes);
    int  get_fd() const;
};
