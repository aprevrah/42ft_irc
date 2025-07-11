#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <iostream>

#define MAX_EVENTS 20

int main(int argc, char **argv) {
    // TODO: check all return values from system calls
    int server_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_socket_fd == -1) {
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6667);

    bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket_fd, 100);  // TODO: rethink about 100
    int                epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_socket_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &event);
    struct epoll_event events[MAX_EVENTS];
    std::cout << "Everything initilized. Listening ..." << std::endl;
    while (true) {
        std::cout << "epoll waiting" << std::endl;
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            std::cerr << "epoll failed";
            exit(1);
        }
        std::cout << "processing epoll events" << std::endl;
        for (int i = 0; i < num_events; i++) {
            // new connection request on server socket
            if (events[i].data.fd == server_socket_fd) {
                struct sockaddr_in incoming_addr;
                socklen_t          addr_len = sizeof(incoming_addr);
                int                client_fd = accept(server_socket_fd, (struct sockaddr *)&incoming_addr, &addr_len);
                if (client_fd == -1) {
                    perror("accept failed");
                    continue;
                }
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    perror("epoll_ctl failed");
                    exit(1);
                }
                std::cout << "connection accepted: " << client_fd << std::endl;
            }
            // data from existing connection
            else {
                std::cout << "data received" << std::endl;
            }
        }
    }
}