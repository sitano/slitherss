#include "game.hpp"

int main(int argc, char* argv[]) {
    slither_server s;

    uint16_t port = 8080;
    uint16_t snakes = 0;

    if (argc < 2) {
        std::cout << "Usage: slither_server [port] [snakes]" << std::endl;
        return 1;
    }

    if (argc >= 2) {
        const int i = atoi(argv[1]);
        if (i <= 0 || i > 65535) {
            std::cout << "invalid port" << std::endl;
            return 1;
        }

        port = uint16_t(i);
    }

    if (argc >= 3) {
        const int i = std::stoi(argv[2]);
        if (i < 0 || i > 10000) {
            std::cout << "invalid snakes count to spawn" << std::endl;
            return 1;
        }

        snakes = uint16_t(i);
    }

    s.run(port, snakes);

    return 0;
}
