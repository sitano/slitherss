#include "game/sector.h"

#include <algorithm>

bool intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
                        float p2_x, float p2_y, float p3_x, float p3_y) {
  const float s1_x = p1_x - p0_x;
  const float s1_y = p1_y - p0_y;
  const float s2_x = p3_x - p2_x;
  const float s2_y = p3_y - p2_y;

  const float d = (-s2_x * s1_y + s1_x * s2_y);
  static const float epsilon = 0.0001f;
  if (d <= epsilon && d >= -epsilon) {
    return false;
  }

  // todo check is it better to have 2 more mul, then 1 branch
  const float s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
  if (s < 0 || s > d) {
    return false;
  }

  const float t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
  return !(t < 0 || t > d);
}

// line vw, and point p
// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x, float p_y) {
  // Return minimum distance between line segment vw and point p
  const float l2 = distance_squared(v_x, v_y, w_x, w_y);  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0) {  // v == w case
    return distance_squared(p_x, p_y, w_x, w_x);
  }
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line.
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  // We clamp t from [0,1] to handle points outside the segment vw.
  float t = ((p_x - v_x) * (w_x - v_x) + (p_y - v_y) * (w_y - v_y)) / l2;
  t = fmaxf(0, fminf(1, t));
  return distance_squared(p_x, p_y, v_x + t * (w_x - v_x), v_y + t * (w_y - v_y));
}

// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
float fastsqrt(float val) {
  union {
    int tmp;
    float val;
  } u;

  u.val = val;
  u.tmp -= 1 << 23; /* Remove last bit so 1.0 gives 1.0 */
  /* tmp is now an approximation to logbase2(val) */
  u.tmp >>= 1;      /* divide by 2 */
  u.tmp += 1 << 29; /* add 64 to exponent: (e+127)/2 =(e/2)+63, */
  /* that represents (e/2)-64 but we want e/2 */

  return u.val;
}

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
// https://betterexplained.com/articles/understanding-quakes-fast-inverse-square-root/
float fastinvsqrt(float x) {
  union {
    int tmp;
    float val;
  } u;

  const float xhalf = 0.5f * x;
  u.val = x;
  u.tmp = 0x5f3759df - (u.tmp >> 1);
  return u.val * (1.5f - xhalf * u.val * u.val);
}

void BoundBox::Insert(Sector *s) {
  auto fwd_i = std::lower_bound(sectors.begin(), sectors.end(), s);
  if (fwd_i != sectors.end()) {
    sectors.insert(fwd_i, s);
  } else {
    sectors.push_back(s);
  }
}

bool BoundBox::RemoveUnsorted(const std::vector<Sector *>::iterator &i) {
  if (i + 1 != sectors.end()) {
    *i = sectors.back();
    sectors.pop_back();
    return true;
  } else {
    sectors.pop_back();
    return false;
  }
}

bool BoundBox::IsPresent(Sector *s) {
  return std::binary_search(sectors.begin(), sectors.end(), s);
}

void BoundBox::Sort() { std::sort(sectors.begin(), sectors.end()); }

size_t BoundBox::get_sectors_count() { return sectors.size(); }

size_t BoundBox::get_snakes_in_sectors_count() {
  size_t i = 0;
  for (Sector *s : sectors) {
    i += s->snakes.size();
  }
  return i;
}

void Sector::RemoveSnake(snake_id_t id) {
  snakes.erase(
      std::remove_if(
          snakes.begin(), snakes.end(),
          [id](const BoundBox *bb) { return bb->id == id; }), snakes.end());
}

void Sector::Insert(const Food &f) {
  auto fwd_i = std::lower_bound(
      food.begin(), food.end(), f,
      [](const Food &a, const Food &b) { return a.x < b.x; });
  if (fwd_i != food.end()) {
    food.insert(fwd_i, f);
  } else {
    food.push_back(f);
  }
}

void Sector::Remove(const std::vector<Food>::iterator &i) {
  food.erase(i);
}

void Sector::Sort() {
  std::sort(food.begin(), food.end(),
            [](const Food &a, const Food &b) { return a.x < b.x; });
}

std::vector<Food>::iterator Sector::FindClosestFood(uint16_t fx) {
  return std::lower_bound(
      food.begin(), food.end(), Food{fx, 0, 0, 0},
      [](const Food &a, const Food &b) { return a.x < b.x; });
}

void SectorSeq::InitSectors() {
  const size_t len = WorldConfig::sector_count_along_edge *
                     WorldConfig::sector_count_along_edge;
  reserve(len);
  for (size_t i = 0; i < len; i++) {
    push_back(Sector{
        static_cast<uint8_t>(i % WorldConfig::sector_count_along_edge),
        static_cast<uint8_t>(i / WorldConfig::sector_count_along_edge)});
  }
}

size_t SectorSeq::get_index(const uint16_t x, const uint16_t y) {
  return y * WorldConfig::sector_count_along_edge + x;
}

Sector *SectorSeq::get_sector(const uint16_t x, const uint16_t y) {
  return &operator[](get_index(x, y));
}

void snake_bb::insert_sorted_with_reg(Sector *s) {
  Insert(s);
  s->snakes.push_back(this);
}

void view_port::reg_new_sector_if_missing(Sector *s) {
  if (std::find(new_sectors.begin(), new_sectors.end(), s) ==
      new_sectors.end()) {
    new_sectors.push_back(s);
  }
}

void view_port::reg_old_sector_if_missing(Sector *s) {
  if (std::find(old_sectors.begin(), old_sectors.end(), s) ==
      old_sectors.end()) {
    old_sectors.push_back(s);
  }
}

void view_port::insert_sorted_with_delta(Sector *s) {
  Insert(s);
  reg_new_sector_if_missing(s);
}

void snake_bb::update_box_new_sectors(SectorSeq *ss, const float bb_r,
                                      const float new_x, const float new_y,
                                      const float old_x, const float old_y) {
  const int16_t new_sx = static_cast<int16_t>(new_x / WorldConfig::sector_size);
  const int16_t new_sy = static_cast<int16_t>(new_y / WorldConfig::sector_size);
  const int16_t old_sx = static_cast<int16_t>(old_x / WorldConfig::sector_size);
  const int16_t old_sy = static_cast<int16_t>(old_y / WorldConfig::sector_size);
  if (new_sx == old_sx && new_sy == old_sy) {
    return;
  }

  const BoundBoxPos box = {new_x, new_y, bb_r};
  static const int16_t map_width_sectors =
      static_cast<int16_t>(WorldConfig::sector_count_along_edge);

  for (int j = new_sy - 1; j <= new_sy + 1; j++) {
    for (int i = new_sx - 1; i <= new_sx + 1; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        Sector *new_sector = ss->get_sector(i, j);
        if (!IsPresent(new_sector) && new_sector->Intersect(box)) {
          insert_sorted_with_reg(new_sector);
        }
      }
    }
  }
}

void snake_bb::update_box_old_sectors() {
  const size_t prev_len = sectors.size();
  auto i = sectors.begin();
  auto sec_end = sectors.end();
  while (i != sec_end) {
    Sector *sec = *i;
    if (!sec->Intersect(*this)) {
      sec->RemoveSnake(id);
      if (RemoveUnsorted(i)) {
        sec_end = sectors.end();
        continue;
      } else {
        break;
      }
    }
    i++;
  }

  if (prev_len != sectors.size()) {
    Sort();
  }
}

void view_port::update_box_new_sectors(SectorSeq *ss,
                                       const float new_x, const float new_y,
                                       const float old_x, const float old_y) {
  const int16_t new_sx = static_cast<int16_t>(new_x / WorldConfig::sector_size);
  const int16_t new_sy = static_cast<int16_t>(new_y / WorldConfig::sector_size);
  const int16_t old_sx = static_cast<int16_t>(old_x / WorldConfig::sector_size);
  const int16_t old_sy = static_cast<int16_t>(old_y / WorldConfig::sector_size);
  if (new_sx == old_sx && new_sy == old_sy) {
    return;
  }

  static const int16_t map_width_sectors =
      static_cast<int16_t>(WorldConfig::sector_count_along_edge);

  for (int j = new_sy - 3; j <= new_sy + 3; j++) {
    for (int i = new_sx - 3; i <= new_sx + 3; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        Sector *new_sector = ss->get_sector(i, j);
        if (!IsPresent(new_sector) && new_sector->Intersect(*this)) {
          insert_sorted_with_delta(new_sector);
        }
      }
    }
  }
}

void view_port::update_box_old_sectors() {
  const size_t prev_len = sectors.size();
  auto i = sectors.begin();
  auto sec_end = sectors.end();
  while (i != sec_end) {
    Sector *sec = *i;
    if (!sec->Intersect(*this)) {
      reg_old_sector_if_missing(sec);
      if (RemoveUnsorted(i)) {
        sec_end = sectors.end();
        continue;
      } else {
        break;
      }
    }
    i++;
  }

  if (prev_len != sectors.size()) {
    Sort();
  }
}
