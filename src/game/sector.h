#ifndef SRC_GAME_SECTOR_H_
#define SRC_GAME_SECTOR_H_

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "game/config.h"
#include "game/food.h"

struct snake;
struct sector;

/**
 * http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
 *
 * FWIW, the following function (in C) both detects line intersections and determines the intersection point.
 * It is based on an algorithm in Andre LeMothe's "Tricks of the Windows Game Programming Gurus".
 * It's not dissimilar to some of the algorithm's in other answers (e.g. Gareth's).
 * LeMothe then uses Cramer's Rule (don't ask me) to solve the equations themselves.
 *
 * I can attest that it works in my feeble asteroids clone, and seems to deal correctly with the edge cases
 * described in other answers by Elemental, Dan and Wodzu. It's also probably faster than the code posted by
 * KingNestor because it's all multiplication and division, no square roots!
 */
bool intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
                        float p2_x, float p2_y, float p3_x, float p3_y);

// center, point, radius
bool intersect_circle(float c_x, float c_y, float p_x, float p_y, float r);

// line vw, and point p
// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x,
                       float p_y);

// points p0, p1
float distance_squared(float p0_x, float p0_y, float p1_x, float p1_y);

// points p0, p1
uint32_t distance_squared(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y);

// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
float fastsqrt(float val);

// float fastinvsqrt(float x);

struct bb_pos {
  float x;
  float y;
  float r;  // squared radius

  bb_pos() = default;
  bb_pos(const bb_pos &p) : x(p.x), y(p.y), r(p.r) {}
  bb_pos(float in_x, float in_y, float in_r) : x(in_x), y(in_y), r(in_r) {}

  inline bool intersect(const bb_pos &bb2) const {
    const float dx = x - bb2.x;
    const float dy = y - bb2.y;
    const float r2 = r + bb2.r;
    return dx * dx + dy * dy <= r2 * r2;
  }
};

struct bb : bb_pos {
  snake_id_t id;
  const snake *snake_ptr;
  std::vector<sector *> m_sectors;

  bb() = default;
  bb(bb_pos in_pos, uint16_t in_id, const snake *in_ptr,
     std::vector<sector *> in_sectors)
      : bb_pos(in_pos), id(in_id), snake_ptr(in_ptr), m_sectors(in_sectors) {}

  void insert_sorted(sector *s);
  bool remove_sector_unsorted(const std::vector<sector *>::iterator &i);
  bool binary_search(sector *s);
  void sort();

  size_t get_sectors_count();
  size_t get_snakes_in_sectors_count();
};

struct sector {
  uint8_t x;
  uint8_t y;

  bb_pos box;

  std::vector<bb *> m_snakes;
  std::vector<Food> m_food;

  sector(uint8_t in_x, uint8_t in_y) : x(in_x), y(in_y) {
    static const uint16_t half = WorldConfig::sector_size / 2;
    static constexpr float r = WorldConfig::sector_diag_size / 2.0f;

    box = {1.0f * (WorldConfig::sector_size * x + half),
           1.0f * (WorldConfig::sector_size * y + half), r};
  }

  inline bool intersect(const bb_pos &box2) const {
    return box.intersect(box2);
  }

  void insert_sorted(const Food &f);
  void remove_food(const std::vector<Food>::iterator &i);
  std::vector<Food>::iterator find_closest_food(uint16_t fx);
  void sort();

  void remove_snake(snake_id_t id);
};

class sectors : public std::vector<sector> {
 public:
  sectors() : std::vector<sector>() {}

  void init_sectors();
  size_t get_index(const uint16_t x, const uint16_t y);
  sector *get_sector(const uint16_t x, const uint16_t y);
};

struct snake_bb : bb {
  snake_bb() = default;
  snake_bb(bb_pos in_pos, uint16_t in_id, const snake *in_ptr,
           const std::vector<sector *> &in_sectors)
      : bb(in_pos, in_id, in_ptr, in_sectors) {}
  explicit snake_bb(bb in)
      : bb({in.x, in.y, in.r}, in.id, in.snake_ptr, in.m_sectors) {}

  void insert_sorted_with_reg(sector *s);
  void update_box_new_sectors(sectors *ss, const float bb_r,
                              const float new_x, const float new_y,
                              const float old_x, const float old_y);
  void update_box_old_sectors();
};

struct view_port : bb {
  std::vector<sector *> new_sectors;
  std::vector<sector *> old_sectors;

  view_port() = default;
  view_port(bb_pos in_pos, uint16_t in_id, const snake *in_ptr,
            const std::vector<sector *> &in_sectors)
      : bb(in_pos, in_id, in_ptr, in_sectors) {}
  explicit view_port(bb in)
      : bb({in.x, in.y, in.r}, in.id, in.snake_ptr, in.m_sectors) {}

  void reg_new_sector_if_missing(sector *s);
  void reg_old_sector_if_missing(sector *s);

  void insert_sorted_with_delta(sector *s);

  void update_box_new_sectors(sectors *ss,
                              const float new_x, const float new_y,
                              const float old_x, const float old_y);
  void update_box_old_sectors();
};

#endif  // SRC_GAME_SECTOR_H_
