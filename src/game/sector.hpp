#ifndef SLITHER_GAME_SECTOR_HPP
#define SLITHER_GAME_SECTOR_HPP

#include "food.hpp"

#include <cstdint>
#include <vector>

struct snake;

struct snake_bb {
    float x;
    float y;
    float r;

    const snake *m_snake;
};

struct sector {
    uint8_t x;
    uint8_t y;

    std::vector<snake_bb> m_snakes;
    std::vector<food> m_food;
};

#endif //SLITHER_GAME_SECTOR_HPP
