#include "game.hpp"

int main(int argc, char* argv[]) {
    slither_server s;

    uint16_t port = 8080;

    if (argc < 2) {
        std::cout << "Usage: slither_server [port]" << std::endl;
        return 1;
    }

    if (argc >= 2) {
        int i = atoi(argv[1]);
        if (i <= 0 || i > 65535) {
            std::cout << "invalid port" << std::endl;
            return 1;
        }

        port = uint16_t(i);
    }

    s.run(port);
    return 0;
}
