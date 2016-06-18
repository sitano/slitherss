#ifndef SLITHER_GAME_FOOD_HPP
#define SLITHER_GAME_FOOD_HPP

#include <cstdint>

struct food {
    uint16_t x;
    uint16_t y;

    uint8_t size;
    uint8_t color; // at least 28 types of colors
};

#endif //SLITHER_GAME_FOOD_HPP
