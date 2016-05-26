#ifndef SLITHER_SERVER_FOOD_HPP
#define SLITHER_SERVER_FOOD_HPP

#include <cstdint>

struct food {
    uint16_t x;
    uint16_t y;

    uint8_t size;
    uint8_t color;
};

#endif //SLITHER_SERVER_FOOD_HPP
