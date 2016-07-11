#ifndef SRC_GAME_FOOD_H_
#define SRC_GAME_FOOD_H_

#include <cstdint>

struct Food {
  uint16_t x;
  uint16_t y;

  uint8_t size;
  uint8_t color;  // at least 28 types of colors
};

#endif  // SRC_GAME_FOOD_H_
