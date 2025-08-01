#include "Server.hpp"

#include <cerrno>
#include <iostream>

// Definition of static member variable
int Server::last_signal = 0;

Server::Server(const int port, const std::string password)
    : server_socket_fd(-1), epoll_fd(-1), port(port), password(password) {}

Server::~Server() {
    if (server_socket_fd > 0) {
        close(server_socket_fd);
    }
    if (epoll_fd > 0) {
        close(epoll_fd);
    }
}

void Server::signal_handler(int signal) {
    last_signal = signal;
}

bool Server::is_correct_password(std::string input) {
    return password == input;
}

void Server::disconnect_client(int client_fd, std::string reason) {
    chan_man.quit_all_channels(clients.at(client_fd), reason);
    clients.at(client_fd).send_response("ERROR :" + reason);
    clients.erase(client_fd);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) == -1) {
        perror("epoll_ctl failed");
    }
    if (close(client_fd) == -1) {
        perror("close failed");
    }
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
        throw std::runtime_error("accept failed");
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
        close(client_fd);
        throw std::runtime_error("fcntl failed");
    }
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
        close(client_fd);
        throw std::runtime_error("epoll_ctl failed");
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
                throw std::runtime_error("read failed");
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

void Server::start() {
    init();
    log_msg(INFO, "Everything initialized. Listening on port " + to_string(port) + ".");
    run();
    log_msg(INFO, "Shutting down");
    disconnect_all_clients();
}

void Server::init() {
    struct sigaction sa;
    sa.sa_handler = Server::signal_handler;
    if (sigemptyset(&sa.sa_mask) == -1) {
        throw std::runtime_error("sigemptyset failed");
    }
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGQUIT, &sa, NULL) == -1) {
        throw std::runtime_error("sigaction failed");
    }

    server_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_socket_fd == -1) {
        throw std::runtime_error("socket failed");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(this->port);

    if (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw std::runtime_error(std::string("bind failed: ") + std::strerror(errno));
    }
    if (listen(server_socket_fd, 100) == -1) {  // TODO: rethink about 100
        throw std::runtime_error("listen failed");
    }
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = server_socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &event) == -1) {
        throw std::runtime_error("epoll_ctl failed");
    }
}

void Server::run() {
    struct epoll_event events[MAX_EVENTS];
    while (true) {
        log_msg(DEBUG, "epoll waiting");
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (last_signal == SIGINT || last_signal == SIGQUIT) {
            break;
        }
        if (num_events == -1 && errno != EINTR) {
            log_msg(ERROR, std::string("epoll failed: ") + strerror(errno));
            break;
        }
        try {
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
        } catch (std::bad_alloc& e) {
            log_msg(ERROR, e.what());
            log_msg(ERROR, "Skipping received data because of failed allocation.");
        } catch (std::exception& e) {
            log_msg(ERROR, e.what());
            log_msg(ERROR, "Skipping received data because of an exception.");
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

void Server::disconnect_all_clients() {
    while (!clients.empty()) {
        disconnect_client(clients.begin()->first, "server shutting down");
    }
}

Client* Server::get_client_by_nick(const std::string nick) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.get_nickname() == nick) {
            return &it->second;
        }
    }
    return NULL;
}