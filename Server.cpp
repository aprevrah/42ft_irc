#include "Server.hpp"

#include <cerrno>
#include <iostream>

Server::Server(const int port, const std::string password) : port(port), password(password) {}

Server::Server(const Server& other) : port(other.port), password(other.password) {}

Server& Server::operator=(const Server& other) {
    (void)other;
    return *this;
}

Server::~Server() {}

bool Server::is_correct_password(std::string input) {
    return password == input;
}

void Server::disconnect_client(int client_fd, std::string reason) {
    chan_man.quit_all_channels(clients[client_fd], reason);
    // I couldn't use the send_numeric_response() here, because ERROR is not an int
    // TODO: maybe we should generalize send_numeric_response() and accept a string as cmd
    clients[client_fd].send_response("ERROR :" + reason);
    clients.erase(client_fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    close(client_fd);
    log_msg(INFO, "Client " + to_string(client_fd) + " disconnected");
}

void Server::handle_new_connection() {
    // TODO: test what happens if two new connection come at the same time
    // (to test this you probaly have to insert a wait into the loop)
    struct sockaddr_in incoming_addr;
    socklen_t          addr_len = sizeof(incoming_addr);
    int                client_fd = accept(server_socket_fd, (struct sockaddr*)&incoming_addr, &addr_len);
    if (client_fd == -1) {
        perror("accept failed");
        return;
    }
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
        perror("epoll_ctl failed");
        exit(1);
    }
    clients[client_fd] = Client(client_fd, this);
    log_msg(INFO, "connection accepted: " + to_string(client_fd));
}
void Server::handle_received_data(int client_fd) {
    char    read_buffer[32];
    ssize_t bytes_read = 1;
    while (bytes_read > 0) {
        memset(read_buffer, 0, sizeof(read_buffer));
        bytes_read = read(client_fd, read_buffer, sizeof(read_buffer) - 1);
        if (bytes_read <= 0) {
            // EAGAIN or EWOULDBLOCK are expected when there is currently nothing left to read, but the connection is
            // still connected. So no error message in this case.
            if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("read");
            }
            // if read returns 0, it means that the client has disconnected
            if (bytes_read == 0) {
                disconnect_client(client_fd, "client closed the connection");
                return;
            }
            break;
        }
        log_msg(DEBUG, "data received: '" + std::string(read_buffer) + "'");
        int status = clients.at(client_fd).add_to_buffer(std::string(read_buffer));
        if (status == CLIENT_DISCONNECTED) {
            return;
        }
    }
}

void Server::run() {
    // TODO: check all return values from system calls
    server_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_socket_fd == -1) {
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(this->port);

    bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket_fd, 100);  // TODO: rethink about 100
    epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_socket_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &event);
    struct epoll_event events[MAX_EVENTS];
    log_msg(INFO, "Everything initialized. Listening on port " + to_string(port) + ".");

    while (true) {
        log_msg(DEBUG, "epoll waiting");
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            log_msg(ERROR, "epoll failed");
            exit(1);
        }
        log_msg(DEBUG, "processing epoll events");
        for (int i = 0; i < num_events; i++) {
            // new connection request on server socket
            if (events[i].data.fd == server_socket_fd) {
                handle_new_connection();
            }
            // data from existing connection
            else {
                handle_received_data(events[i].data.fd);
            }
        }
    }
}

bool Server::is_nick_available(const std::string nick) {
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.get_nickname() == nick) {
            return false;
        }
    }
    return true;
}

Client* Server::get_client_by_nick(const std::string nick) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.get_nickname() == nick) {
            return &it->second;
        }
    }
    return NULL;
}