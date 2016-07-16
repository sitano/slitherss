#ifndef SRC_GAME_FOOD_H_
#define SRC_GAME_FOOD_H_

#include <cstdint>
#include <vector>

struct Food {
  uint16_t x;
  uint16_t y;

  uint8_t size;
  uint8_t color;  // at least 28 types of colors

  // some compilers would not auto pad size of this struct,
  // and valgrind will blow this up when reading from 64 bit reg
  uint16_t _padding = 0;

  Food() = default;
  Food(uint16_t in_x, uint16_t in_y, uint8_t in_size, uint8_t in_color)
      : x(in_x), y(in_y), size(in_size), color(in_color) { }
};

typedef std::vector<Food> FoodSeq;
typedef std::vector<Food>::iterator FoodSeqIter;

#endif  // SRC_GAME_FOOD_H_
