#ifndef SRC_GAME_SECTOR_H_
#define SRC_GAME_SECTOR_H_

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "game/config.h"
#include "game/food.h"

struct Snake;
struct Sector;

typedef std::vector<Sector *> SectorVec;

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

// line vw, and point p
// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x, float p_y);

// points p0, p1
inline float distance_squared(float p0_x, float p0_y, float p1_x, float p1_y) {
  const float dx = p0_x - p1_x;
  const float dy = p0_y - p1_y;
  return dx * dx + dy * dy;
}

// points p0, p1
inline int32_t distance_squared(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y) {
  const int32_t dx = p0_x - p1_x;
  const int32_t dy = p0_y - p1_y;
  return dx * dx + dy * dy;
}

// center, point, radius
inline bool intersect_circle(float c_x, float c_y, float p_x, float p_y, float r) {
  return distance_squared(c_x, c_y, p_x, p_y) <= r * r;
}

// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
float fastsqrt(float val);

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
// https://betterexplained.com/articles/understanding-quakes-fast-inverse-square-root/
float fastinvsqrt(float x);

struct BoundBoxPos {
  float x;
  float y;
  float r;

  BoundBoxPos() = default;
  BoundBoxPos(const BoundBoxPos &p) : x(p.x), y(p.y), r(p.r) {}
  BoundBoxPos(float in_x, float in_y, float in_r) : x(in_x), y(in_y), r(in_r) {}

  inline bool Intersect(const BoundBoxPos &bb2) const {
    return intersect_circle(x, y, bb2.x, bb2.y, r + bb2.r);
  }
};

class BoundBox : public BoundBoxPos {
 public:
  snake_id_t id;
  const Snake *snake;
  SectorVec sectors;

  BoundBox() = default;
  BoundBox(BoundBoxPos in_pos, uint16_t in_id, const Snake *in_snake, SectorVec in_sectors)
      : BoundBoxPos(in_pos), id(in_id), snake(in_snake), sectors(in_sectors) {}

  void Insert(Sector *s);
  bool RemoveUnsorted(const std::vector<Sector *>::iterator &i);
  bool IsPresent(Sector *s);
  void Sort();

  size_t get_sectors_count();
  size_t get_snakes_in_sectors_count();
};

class Sector {
 public:
  uint8_t x;
  uint8_t y;

  BoundBoxPos box;

  std::vector<BoundBox *> snakes;
  std::vector<Food> food;

  Sector(uint8_t in_x, uint8_t in_y) : x(in_x), y(in_y) {
    static const uint16_t half = WorldConfig::sector_size / 2;
    static constexpr float r = WorldConfig::sector_diag_size / 2.0f;

    box = {1.0f * (WorldConfig::sector_size * x + half),
           1.0f * (WorldConfig::sector_size * y + half), r};
  }

  inline bool Intersect(const BoundBoxPos &box2) const {
    return box.Intersect(box2);
  }

  void Insert(const Food &f);
  void Remove(const std::vector<Food>::iterator &i);
  std::vector<Food>::iterator FindClosestFood(uint16_t fx);
  void Sort();

  void RemoveSnake(snake_id_t id);
};

class SectorSeq : public std::vector<Sector> {
 public:
  SectorSeq() : std::vector<Sector>() {}

  void InitSectors();

  size_t get_index(const uint16_t x, const uint16_t y);
  Sector *get_sector(const uint16_t x, const uint16_t y);
};

class snake_bb : public BoundBox {
 public:
  snake_bb() = default;
  snake_bb(BoundBoxPos in_pos, uint16_t in_id, const Snake *in_ptr, const std::vector<Sector *> &in_sectors)
      : BoundBox(in_pos, in_id, in_ptr, in_sectors) {}
  explicit snake_bb(BoundBox in) : BoundBox({in.x, in.y, in.r}, in.id, in.snake, in.sectors) {}

  void insert_sorted_with_reg(Sector *s);
  void update_box_new_sectors(SectorSeq *ss, const float bb_r,
                              const float new_x, const float new_y,
                              const float old_x, const float old_y);
  void update_box_old_sectors();
};

class view_port : public BoundBox {
 public:
  std::vector<Sector *> new_sectors;
  std::vector<Sector *> old_sectors;

  view_port() = default;
  view_port(BoundBoxPos in_pos, uint16_t in_id, const Snake *in_ptr,
            const std::vector<Sector *> &in_sectors)
      : BoundBox(in_pos, in_id, in_ptr, in_sectors) {}
  explicit view_port(BoundBox in)
      : BoundBox({in.x, in.y, in.r}, in.id, in.snake, in.sectors) {}

  void reg_new_sector_if_missing(Sector *s);
  void reg_old_sector_if_missing(Sector *s);

  void insert_sorted_with_delta(Sector *s);

  void update_box_new_sectors(SectorSeq *ss,
                              const float new_x, const float new_y,
                              const float old_x, const float old_y);
  void update_box_old_sectors();
};

#endif  // SRC_GAME_SECTOR_H_
