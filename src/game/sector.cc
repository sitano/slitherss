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

// center, point, radius
bool intersect_circle(float c_x, float c_y, float p_x, float p_y, float r) {
  return distance_squared(c_x, c_y, p_x, p_y) <= r * r;
}

// line vw, and point p
// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x,
                       float p_y) {
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

// points p0, p1
float distance_squared(float p0_x, float p0_y, float p1_x, float p1_y) {
  const float dx = p0_x - p1_x;
  const float dy = p0_y - p1_y;
  return dx * dx + dy * dy;
}

uint32_t distance_squared(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x,
                          uint16_t p1_y) {
  const int32_t dx = p0_x - p1_x;
  const int32_t dy = p0_y - p1_y;
  return dx * dx + dy * dy;
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

/*
float fastinvsqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i>>1);
    x = *(float*)&i;
    return x*(1.5f - xhalf*x*x);
}
*/

void bb::insert_sorted(sector *s) {
  auto fwd_i = std::lower_bound(m_sectors.begin(), m_sectors.end(), s);
  if (fwd_i != m_sectors.end()) {
    m_sectors.insert(fwd_i, s);
  } else {
    m_sectors.push_back(s);
  }
}

bool bb::remove_sector_unsorted(const std::vector<sector *>::iterator &i) {
  if (i + 1 != m_sectors.end()) {
    *i = m_sectors.back();
    m_sectors.pop_back();
    return true;
  } else {
    m_sectors.pop_back();
    return false;
  }
}

bool bb::binary_search(sector *s) {
  return std::binary_search(m_sectors.begin(), m_sectors.end(), s);
}

void bb::sort() { std::sort(m_sectors.begin(), m_sectors.end()); }

size_t bb::get_sectors_count() { return m_sectors.size(); }

size_t bb::get_snakes_in_sectors_count() {
  size_t i = 0;
  for (sector *s : m_sectors) {
    i += s->m_snakes.size();
  }
  return i;
}

void sector::remove_snake(snake_id_t id) {
  m_snakes.erase(std::remove_if(m_snakes.begin(), m_snakes.end(),
                                [id](const bb *bb) { return bb->id == id; }));
}

void sector::insert_sorted(const food &f) {
  auto fwd_i =
      std::lower_bound(m_food.begin(), m_food.end(), f,
                       [](const food &a, const food &b) { return a.x < b.x; });
  if (fwd_i != m_food.end()) {
    m_food.insert(fwd_i, f);
  } else {
    m_food.push_back(f);
  }
}

void sector::remove_food(const std::vector<food>::iterator &i) {
  m_food.erase(i);
}

void sector::sort() {
  std::sort(m_food.begin(), m_food.end(),
            [](const food &a, const food &b) { return a.x < b.x; });
}

std::vector<food>::iterator sector::find_closest_food(uint16_t fx) {
  return std::lower_bound(
      m_food.begin(), m_food.end(), food{fx, 0, 0, 0},
      [](const food &a, const food &b) { return a.x < b.x; });
}

void sectors::init_sectors() {
  const size_t len = world_config::sector_count_along_edge *
                     world_config::sector_count_along_edge;
  reserve(len);
  for (size_t i = 0; i < len; i++) {
    push_back(sector{
        static_cast<uint8_t>(i % world_config::sector_count_along_edge),
        static_cast<uint8_t>(i / world_config::sector_count_along_edge)});
  }
}

size_t sectors::get_index(const uint16_t x, const uint16_t y) {
  return y * world_config::sector_count_along_edge + x;
}

sector *sectors::get_sector(const uint16_t x, const uint16_t y) {
  return &operator[](get_index(x, y));
}

void snake_bb::insert_sorted_with_reg(sector *s) {
  insert_sorted(s);
  s->m_snakes.push_back(this);
}

void view_port::reg_new_sector_if_missing(sector *s) {
  if (std::find(new_sectors.begin(), new_sectors.end(), s) ==
      new_sectors.end()) {
    new_sectors.push_back(s);
  }
}

void view_port::reg_old_sector_if_missing(sector *s) {
  if (std::find(old_sectors.begin(), old_sectors.end(), s) ==
      old_sectors.end()) {
    old_sectors.push_back(s);
  }
}

void view_port::insert_sorted_with_delta(sector *s) {
  insert_sorted(s);
  reg_new_sector_if_missing(s);
}

void snake_bb::update_box_new_sectors(sectors *ss, const float bb_r,
                                      const float new_x, const float new_y,
                                      const float old_x, const float old_y) {
  const int16_t new_sx = static_cast<int16_t>(new_x / world_config::sector_size);
  const int16_t new_sy = static_cast<int16_t>(new_y / world_config::sector_size);
  const int16_t old_sx = static_cast<int16_t>(old_x / world_config::sector_size);
  const int16_t old_sy = static_cast<int16_t>(old_y / world_config::sector_size);
  if (new_sx == old_sx && new_sy == old_sy) {
    return;
  }

  const bb_pos box = {new_x, new_y, bb_r};
  static const int16_t map_width_sectors =
      static_cast<int16_t>(world_config::sector_count_along_edge);

  for (int j = new_sy - 1; j <= new_sy + 1; j++) {
    for (int i = new_sx - 1; i <= new_sx + 1; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        sector *new_sector = ss->get_sector(i, j);
        if (!binary_search(new_sector) && new_sector->intersect(box)) {
          insert_sorted_with_reg(new_sector);
        }
      }
    }
  }
}

void snake_bb::update_box_old_sectors() {
  const size_t prev_len = m_sectors.size();
  auto i = m_sectors.begin();
  auto sec_end = m_sectors.end();
  while (i != sec_end) {
    sector *sec = *i;
    if (!sec->intersect(*this)) {
      sec->remove_snake(id);
      if (remove_sector_unsorted(i)) {
        sec_end = m_sectors.end();
        continue;
      } else {
        break;
      }
    }
    i++;
  }

  if (prev_len != m_sectors.size()) {
    sort();
  }
}

void view_port::update_box_new_sectors(sectors *ss,
                                       const float new_x, const float new_y,
                                       const float old_x, const float old_y) {
  const int16_t new_sx =
      static_cast<int16_t>(new_x / world_config::sector_size);
  const int16_t new_sy =
      static_cast<int16_t>(new_y / world_config::sector_size);
  const int16_t old_sx =
      static_cast<int16_t>(old_x / world_config::sector_size);
  const int16_t old_sy =
      static_cast<int16_t>(old_y / world_config::sector_size);

  if (new_sx == old_sx && new_sy == old_sy) {
    return;
  }

  static const int16_t map_width_sectors =
      static_cast<int16_t>(world_config::sector_count_along_edge);

  for (int j = new_sy - 3; j <= new_sy + 3; j++) {
    for (int i = new_sx - 3; i <= new_sx + 3; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        sector *new_sector = ss->get_sector(i, j);
        if (!binary_search(new_sector) && new_sector->intersect(*this)) {
          insert_sorted_with_delta(new_sector);
        }
      }
    }
  }
}

void view_port::update_box_old_sectors() {
  const size_t prev_len = m_sectors.size();
  auto i = m_sectors.begin();
  auto sec_end = m_sectors.end();
  while (i != sec_end) {
    sector *sec = *i;
    if (!sec->intersect(*this)) {
      reg_old_sector_if_missing(sec);
      if (remove_sector_unsorted(i)) {
        sec_end = m_sectors.end();
        continue;
      } else {
        break;
      }
    }
    i++;
  }

  if (prev_len != m_sectors.size()) {
    sort();
  }
}
