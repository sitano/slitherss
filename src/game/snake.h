#ifndef SRC_GAME_SNAKE_H_
#define SRC_GAME_SNAKE_H_

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "game/config.h"
#include "game/sector.h"

enum snake_changes_t : uint8_t {
  change_pos = 1,
  change_angle = 1 << 1,
  change_wangle = 1 << 2,
  change_speed = 1 << 3,
  change_fullness = 1 << 4,
  change_dying = 1 << 5,
  change_dead = 1 << 6
};

struct Body {
  float x;
  float y;

  inline void From(const Body &p) {
    x = p.x;
    y = p.y;
  }

  inline void Offset(float dx, float dy) {
    x += dx;
    y += dy;
  }

  inline float DistanceSquared(float dx, float dy) const {
    const float a = x - dx;
    const float b = y - dy;
    return a * a + b * b;
  }
};

typedef std::vector<Body> BodySeq;
typedef std::vector<Body>::const_iterator BodySeqCIter;

class Snake : public std::enable_shared_from_this<Snake> {
 public:
  typedef std::shared_ptr<Snake> Ptr;

  snake_id_t id;

  uint8_t skin;
  uint8_t update;
  bool acceleration;
  bool bot;

  std::string name;

  // pixels / seconds, base ~185 [px/s]
  uint16_t speed;

  float angle;
  float wangle;

  // 0 - 100, 0 - hungry, 100 - full
  uint16_t fullness;

  SnakeBoundBox sbb;
  ViewPort vp;
  BodySeq parts;
  FoodSeq eaten;
  FoodSeq spawn;
  size_t clientPartsIndex;

  bool Tick(long dt, SectorSeq *ss);
  void TickAI(long frames);
  void UpdateBoxCenter();
  void UpdateBoxRadius();
  void UpdateSnakeConsts();
  void InitBoxNewSectors(SectorSeq *ss);
  void UpdateEatenFood(SectorSeq *ss);

  bool Intersect(BoundBoxPos foe) const;
  bool Intersect(BoundBoxPos foe, BodySeqCIter prev, BodySeqCIter iter, BodySeqCIter end) const;

  void IncreaseSnake(uint16_t volume);
  void DecreaseSnake(uint16_t volume);
  void SpawnFood(Food f);

  void on_dead_food_spawn(SectorSeq *ss, std::function<float()> next_randomf);
  void on_food_eaten(Food f);

  float get_snake_scale() const;
  float get_snake_body_part_radius() const;
  uint16_t get_snake_score() const;

  inline const Body &get_head() const { return parts[0]; }
  inline float get_head_x() const { return parts[0].x; }
  inline float get_head_y() const { return parts[0].y; }
  inline float get_head_dx() const { return parts[0].x - parts[1].x; }
  inline float get_head_dy() const { return parts[0].y - parts[1].y; }

  std::shared_ptr<Snake> get_ptr();
  BoundBox get_new_box() const;

  static constexpr float spangdv = 4.8f;
  static constexpr float nsp1 = 5.39f;
  static constexpr float nsp2 = 0.4f;
  static constexpr float nsp3 = 14.0f;

  static const uint16_t base_move_speed = 185;  // pixel in second (convert:  1000*sp/32)
  static const uint16_t boost_speed = 448;  // pixel in second (convert:  1000*sp/32)
  static const uint16_t speed_acceleration = 1000;  // pixel in second

  static constexpr float prey_angular_speed = 3.5f;  // radian in second (convert: 1000ms/8ms * ang[rad])
  static constexpr float snake_angular_speed = 4.125f;  // radian in second (convert:  1000ms/8ms * ang[rad])
  static constexpr float snake_tail_k = 0.43f;  // snake tail rigidity (0 .. 0.5]

  static const int parts_skip_count = 3;
  static const int parts_start_move_count = 4;
  static constexpr float tail_step_distance = 24.0f;  // tail step eval for step dist = 42, k = 0.43
  static constexpr float rot_step_angle = 1.0f * WorldConfig::move_step_distance /
    boost_speed * snake_angular_speed;  // radians step per max acc resolution time
  static const long rot_step_interval = static_cast<long>(1000.0f * rot_step_angle / snake_angular_speed);
  static const long ai_step_interval = 1000;

  static constexpr float f_pi = 3.14159265358979323846f;
  static constexpr float f_2pi = 2.0f * f_pi;

  inline static float normalize_angle(float ang) {
    return ang - f_2pi * floorf(ang / f_2pi);
  }

 private:
  long mov_ticks = 0;
  long rot_ticks = 0;
  long ai_ticks = 0;

 private:
  float gsc = 0.0f;  // snake scale 0.5f + 0.4f / fmaxf(1.0f, 1.0f * (parts.size() - 1 + 16) / 36.0f)
  float sc = 0.0f;  // 106th length on snake, min 1, start from 6. Math.min(6, 1 + (f.sct - 2) / 106)
  float scang = 0.0f;  // .13 + .87 * Math.pow((7 - f.sc) / 6, 2)
  float ssp = 0.0f;    // nsp1 + nsp2 * f.sc;
  float fsp = 0.0f;    // f.ssp + .1;
  // snake body part radius, in screen coords it is:
  // - gsc * sbpr * 52 / 32 inner r, and
  // - gsc * sbpr * 62 / 32 for outer r.
  // thus, for sbpr 14.5, inner 21.20, outer 25.28
  float sbpr = 0.0f;
};

typedef std::vector<Snake *> SnakeVec;
typedef std::unordered_map<snake_id_t, std::shared_ptr<Snake>> SnakeMap;
typedef SnakeMap::iterator SnakeMapIter;
typedef std::vector<snake_id_t> Ids;

#endif  // SRC_GAME_SNAKE_H_
