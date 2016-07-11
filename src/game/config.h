#ifndef SRC_GAME_CONFIG_H_
#define SRC_GAME_CONFIG_H_

#include <cstdint>

typedef uint16_t snake_id_t;

struct WorldConfig {
  // generate bots on start count
  uint16_t bots = 0;

  // average snake length on init
  uint16_t snake_average_length = 2;
  uint16_t snake_min_length = 2;

  static const uint16_t game_radius = 21600;
  static const uint16_t max_snake_parts = 411;
  static const uint16_t sector_size = 300;
  // sector_count_along_edge = 144
  static const uint16_t sector_count_along_edge = 2 * game_radius / sector_size;
  static const uint16_t death_radius = game_radius - sector_size;
  // sector_diag_size = 1 + sqrtf(sector_size * sector_size * 2)
  static const uint16_t sector_diag_size = 425;
  static const uint16_t move_step_distance = 42;
};

#endif  // SRC_GAME_CONFIG_H_
