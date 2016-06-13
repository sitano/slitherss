#ifndef SLITHER_GAME_CONFIG_HPP
#define SLITHER_GAME_CONFIG_HPP

#include <cstdint>

typedef uint16_t snake_id_t;

struct world_config {
    static const uint16_t game_radius = 21600;
    static const uint16_t max_snake_parts = 411;
    static const uint16_t sector_size = 300;
    static const uint16_t sector_count_along_edge = 2 * game_radius / sector_size;
    static const uint16_t death_radius = game_radius - sector_size;
    static const uint16_t sector_diag_size = 425; // static_cast<uint16_t>(1 + sqrtf(sector_size * sector_size * 2));
};

#endif //SLITHER_GAME_CONFIG_HPP
