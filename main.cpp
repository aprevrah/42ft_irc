#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>

int main(int argc, char **argv)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socket_fd == -1)
    {
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6667);
    
    bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(socket_fd, 100); //TODO: rethink about 100
    int epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    while (true) {
        epoll_wait(epoll_fd, &event, 1, -1);
        std::cout << "processing epoll events" << std::endl;
        struct sockaddr_in incoming_addr;
        socklen_t addr_len = sizeof(incoming_addr);
        int client_fd = accept(socket_fd, (struct sockaddr*)&incoming_addr, &addr_len);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }
        std::cout << "connection accepted: " << client_fd << std::endl;
    }
}