#ifndef SLITHER_SERVER_SNAKE_HPP
#define SLITHER_SERVER_SNAKE_HPP

#include <cstdint>
#include <vector>
#include <string>

struct body {
    uint32_t x;
    uint32_t y;
};

struct snake {
    uint16_t id;

    std::string name;
    uint8_t color;

    uint32_t x;
    uint32_t y;

    float speed;

    float angle;
    float wangle;
    float eangle;

    float ehang;
    float wehang;

    float fullness;

    std::vector<body> parts;
};

#endif //SLITHER_SERVER_SNAKE_HPP
