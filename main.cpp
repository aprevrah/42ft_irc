#include "Server.hpp"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (argc != 3) {
        std::cout << "Usage: ircsrv <port> <password>" << std::endl;
        return 1;
    }
    Server server(strtol(argv[1], NULL, 10), std::string(argv[2]));
    server.run();
}