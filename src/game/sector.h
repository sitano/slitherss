#ifndef SRC_GAME_SECTOR_H_
#define SRC_GAME_SECTOR_H_

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "game/config.h"
#include "game/food.h"
#include "game/math.h"

class Snake;
class Sector;
class BoundBox;

typedef std::vector<Sector *> SectorVec;
typedef std::vector<Sector *>::iterator SectorIter;
typedef std::vector<BoundBox *> BoundBoxVec;

struct BoundBoxPos {
  float x;
  float y;
  float r;

  BoundBoxPos() = default;
  BoundBoxPos(const BoundBoxPos &p) : x(p.x), y(p.y), r(p.r) {}
  BoundBoxPos(float in_x, float in_y, float in_r) : x(in_x), y(in_y), r(in_r) {}

  inline bool Intersect(const BoundBoxPos &bb2) const {
    return Math::intersect_circle(x, y, bb2.x, bb2.y, r + bb2.r);
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
  bool RemoveUnsorted(const SectorIter &i);
  bool IsPresent(const Sector *s);
  void Sort();

  size_t get_sectors_count();
  size_t get_snakes_in_sectors_count();
};

class Sector {
 public:
  uint8_t x;
  uint8_t y;

  BoundBoxPos box;
  BoundBoxVec snakes;
  FoodSeq food;

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
  void Remove(const FoodSeqIter &i);
  FoodSeqIter FindClosestFood(uint16_t fx);
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

class SnakeBoundBox : public BoundBox {
 public:
  SnakeBoundBox() = default;

  SnakeBoundBox(BoundBoxPos in_pos, uint16_t in_id,
                const Snake *in_ptr, const SectorVec &in_sectors)
      : BoundBox(in_pos, in_id, in_ptr, in_sectors) {}

  explicit SnakeBoundBox(BoundBox in) : BoundBox({in.x, in.y, in.r}, in.id, in.snake, in.sectors) {}

  void InsertSortedWithReg(Sector *s);
  void UpdateBoxNewSectors(SectorSeq *ss, const float bb_r,
                           const float new_x, const float new_y,
                           const float old_x, const float old_y);
  void UpdateBoxOldSectors();
};

class ViewPort : public BoundBox {
 public:
  SectorVec new_sectors;
  SectorVec old_sectors;

  ViewPort() = default;

  ViewPort(BoundBoxPos in_pos, uint16_t in_id, const Snake *in_ptr, const SectorVec &in_sectors)
      : BoundBox(in_pos, in_id, in_ptr, in_sectors) {}

  explicit ViewPort(BoundBox in) : BoundBox({in.x, in.y, in.r}, in.id, in.snake, in.sectors) {}

  void RegNewSectorIfMissing(Sector *s);
  void RegOldSectorIfMissing(Sector *s);

  void InsertSortedWithDelta(Sector *s);

  void UpdateBoxNewSectors(SectorSeq *ss,
                           const float new_x, const float new_y,
                           const float old_x, const float old_y);
  void UpdateBoxOldSectors();
};

#endif  // SRC_GAME_SECTOR_H_
