#ifndef SLITHER_SERVER_SECTOR_HPP
#define SLITHER_SERVER_SECTOR_HPP

#include <cstdint>
#include <vector>

#include "food.hpp"

struct sector {
    uint8_t x;
    uint8_t y;

    std::vector<food> food;
};

#endif //SLITHER_SERVER_SECTOR_HPP
