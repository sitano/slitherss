#ifndef SLITHER_GAME_SECTOR_HPP
#define SLITHER_GAME_SECTOR_HPP

#include "food.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>

struct snake;
struct sector;

struct snake_bb_pos {
    float x;
    float y;
    float r2; // squared radius

    snake_bb_pos() = default;
    snake_bb_pos(const snake_bb_pos &p) : x(p.x), y(p.y), r2(p.r2) {}
    snake_bb_pos(float in_x, float in_y, float in_r2) : x(in_x), y(in_y), r2(in_r2) {}

    inline bool intersect(const snake_bb_pos &bb2) const {
        const float dx = x - bb2.x;
        const float dy = y - bb2.y;
        // x^2 + y^2 <= r^2 = (r1 + r2) ^ 2 <= r1^2 + r2^2 + 2*r1*r2 <= r1^2 + r2^2 + 2*max(r1^2,r2^2)
        const float rr = fmaxf(r2, bb2.r2);
        return dx * dx + dy * dy <= r2 + bb2.r2 + 2.0f * rr;
    }
};

struct snake_bb : snake_bb_pos {
    uint16_t id;
    const snake * ptr;
    std::vector<sector *> sectors;

    snake_bb() = default;
    snake_bb(snake_bb_pos in_pos, uint16_t in_id, const snake * in_ptr, std::vector<sector *> in_sectors) :
        snake_bb_pos(in_pos), id(in_id), ptr(in_ptr), sectors(in_sectors) {}

    inline bool find(sector *s) {
        for (auto &i : sectors) {
            if (i == s) {
                return true;
            }
        }

        return false;
    }

    inline size_t get_sectors_count() {
        return sectors.size();
    }

    size_t get_snakes_in_sectors_count();
};

struct sector {
    uint8_t x;
    uint8_t y;

    std::vector<snake_bb> m_snakes;
    std::vector<food> m_food;

    inline bool intersect(const snake_bb_pos &bb2, const uint16_t sector_size, const uint16_t sector_diag_size) const {
        const snake_bb_pos bb1 = {
                1.0f * (x * sector_size),
                1.0f * (y * sector_size),
                1.0f * (sector_diag_size * sector_diag_size / 4) };
        return bb1.intersect(bb2);
    }
};

#endif //SLITHER_GAME_SECTOR_HPP
