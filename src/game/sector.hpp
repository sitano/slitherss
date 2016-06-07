#ifndef SLITHER_GAME_SECTOR_HPP
#define SLITHER_GAME_SECTOR_HPP

#include <cstdint>
#include <vector>

#include "food.hpp"

struct sector {
    uint8_t x;
    uint8_t y;

    std::vector<food> m_food;
};

#endif //SLITHER_GAME_SECTOR_HPP
