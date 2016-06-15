#include "game.hpp"

int main(const int argc, const char* const argv[]) {
    slither_server s;

    s.run(parse_command_line(argc, argv));

    return 0;
}
