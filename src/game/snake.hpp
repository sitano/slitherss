#ifndef SLITHER_SERVER_SNAKE_HPP
#define SLITHER_SERVER_SNAKE_HPP

#include <cstdint>
#include <vector>
#include <string>

struct Body {
    uint32_t x;
    uint32_t y;
};

struct Snake {
    uint16_t id;

    std::string name;

    uint32_t x;
    uint32_t y;

    std::vector<Body> parts;
};

#endif //SLITHER_SERVER_SNAKE_HPP
