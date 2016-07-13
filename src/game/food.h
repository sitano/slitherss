#ifndef SRC_GAME_FOOD_H_
#define SRC_GAME_FOOD_H_

#include <cstdint>
#include <vector>

struct Food {
  uint16_t x;
  uint16_t y;

  uint8_t size;
  uint8_t color;  // at least 28 types of colors
};

typedef std::vector<Food> FoodSeq;
typedef std::vector<Food>::iterator FoodSeqIter;

#endif  // SRC_GAME_FOOD_H_
