#ifndef SLITHER_GAME_SNAKE_HPP
#define SLITHER_GAME_SNAKE_HPP

#include <cstdint>
#include <vector>
#include <string>

struct body {
    float x;
    float y;
};

struct snake {
    uint16_t id;

    std::string name;
    uint8_t color;

    float x;
    float y;

    float speed;

    float angle;
    float wangle;

    float fullness;

    std::vector<body> parts;
};

#endif //SLITHER_GAME_SNAKE_HPP
