#include "game/snake.h"

#include <iostream>

#include "game/math.h"

bool Snake::Tick(long dt, SectorSeq *ss) {
  uint8_t changes = 0;

  if (update & (change_dying | change_dead)) {
    return false;
  }

  if (bot) {
    ai_ticks += dt;
    if (ai_ticks > ai_step_interval) {
      const long frames = ai_ticks / ai_step_interval;
      const long frames_ticks = frames * ai_step_interval;

      TickAI(frames);

      ai_ticks -= frames_ticks;
    }
  }

  // rotation
  if (angle != wangle) {
    rot_ticks += dt;
    if (rot_ticks >= rot_step_interval) {
      const long frames = rot_ticks / rot_step_interval;
      const long frames_ticks = frames * rot_step_interval;
      const float rotation = snake_angular_speed * frames_ticks / 1000.0f;
      float dAngle = Math::normalize_angle(wangle - angle);

      if (dAngle > Math::f_pi) {
        dAngle -= Math::f_2pi;
      }

      if (fabsf(dAngle) < rotation) {
        angle = wangle;
      } else {
        angle += rotation * (dAngle > 0 ? 1.0f : -1.0f);
      }

      angle = Math::normalize_angle(angle);

      changes |= change_angle;
      rot_ticks -= frames_ticks;
    }
  }

  // movement
  mov_ticks += dt;
  const long mov_frame_interval = 1000 * WorldConfig::move_step_distance / speed;
  if (mov_ticks >= mov_frame_interval) {
    const long frames = mov_ticks / mov_frame_interval;
    const long frames_ticks = frames * mov_frame_interval;
    const float move_dist = speed * frames_ticks / 1000.0f;
    const size_t len = parts.size();

    // move head
    Body &head = parts[0];
    Body prev = head;
    head.x += cosf(angle) * move_dist;
    head.y += sinf(angle) * move_dist;

    sbb.UpdateBoxNewSectors(ss, WorldConfig::sector_size / 2, head.x, head.y,
                            prev.x, prev.y);
    if (!bot) {
      vp.UpdateBoxNewSectors(ss, head.x, head.y, prev.x, prev.y);
    }

    // bound box
    float bbx = head.x;
    float bby = head.y;

    for (size_t i = 1; i < len && i < parts_skip_count; ++i) {
      const Body old = parts[i];
      parts[i] = prev;
      bbx += prev.x;
      bby += prev.y;
      prev = old;
    }

    // move intermediate
    for (size_t i = parts_skip_count, j = 0; i < len && i < parts_skip_count + parts_start_move_count; ++i) {
      Body &pt = parts[i];
      const Body last = parts[i - 1];
      const Body old = pt;

      pt.From(prev);
      const float move_coeff = snake_tail_k * (++j) / parts_start_move_count;
      pt.Offset(move_coeff * (last.x - pt.x), move_coeff * (last.y - pt.y));

      bbx += pt.x;
      bby += pt.y;
      prev = old;
    }

    // move tail
    for (size_t i = parts_skip_count + parts_start_move_count, j = 0; i < len; ++i) {
      Body &pt = parts[i];
      const Body last = parts[i - 1];
      const Body old = pt;

      pt.From(prev);
      pt.Offset(snake_tail_k * (last.x - pt.x), snake_tail_k * (last.y - pt.y));

      // as far as having step dist = 42, k = 0.43, sec. size = 300, this could
      // be 300 / 24.0f, with radius 150
      static const size_t tail_step =
          static_cast<size_t>(WorldConfig::sector_size / tail_step_distance);
      if (j + tail_step >= i) {
        sbb.UpdateBoxNewSectors(ss, WorldConfig::sector_size / 2, pt.x, pt.y,
                                old.x, old.y);
        j = i;
      }

      bbx += pt.x;
      bby += pt.y;
      prev = old;
    }

    changes |= change_pos;

    // update bb
    sbb.x = bbx / parts.size();
    sbb.y = bby / parts.size();
    vp.x = head.x;
    vp.y = head.y;
    UpdateBoxRadius();
    sbb.UpdateBoxOldSectors();
    if (!bot) {
      vp.UpdateBoxOldSectors();
    }
    UpdateEatenFood(ss);

    // update speed
    if (acceleration) {
      if (parts.size() <= 3) {
        acceleration = false;
      } else {
        DecreaseSnake(33);
      }
    }

    const uint16_t wantedSpeed = acceleration ? boost_speed : base_move_speed;
    if (speed != wantedSpeed) {
      const float sgn = wantedSpeed > speed ? 1.0f : -1.0f;
      const uint16_t acc = static_cast<uint16_t>(speed_acceleration * frames_ticks / 1000.0f);
      if (abs(wantedSpeed - speed) <= acc) {
        speed = wantedSpeed;
      } else {
        speed += sgn * acc;
      }
      changes |= change_speed;
    }

    mov_ticks -= frames_ticks;
  }

  if (changes > 0 && changes != update) {
    update |= changes;
    return true;
  }

  return false;
}

std::shared_ptr<Snake> Snake::get_ptr() { return shared_from_this(); }

void Snake::UpdateBoxCenter() {
  float x = 0.0f;
  float y = 0.0f;

  // calculate center mass
  for (const Body &p : parts) {
    x += p.x;
    y += p.y;
  }

  x /= parts.size();
  y /= parts.size();

  sbb.x = x;
  sbb.y = y;

  vp.x = get_head_x();
  vp.y = get_head_y();
}

void Snake::UpdateBoxRadius() {
  // calculate bb radius, len eval for step dist = 42, k = 0.43
  // parts ..  1  ..  2  ..  3   ..  4   ..  5   ..  6   ..  7  .. tail by 24.0f
  float d = 42.0f + 42.0f + 42.0f + 37.7f + 37.7f + 33.0f + 28.5f;

  if (parts.size() > 8) {
    d += tail_step_distance * (parts.size() - 8);
  }

  // reserve 1 step ahead of the snake radius
  sbb.r = (d + WorldConfig::move_step_distance) / 2.0f;

  vp.r = WorldConfig::sector_diag_size * 3.0f;
}

void Snake::InitBoxNewSectors(SectorSeq *ss) {
  Body &head = parts[0];
  sbb.UpdateBoxNewSectors(ss, WorldConfig::sector_size / 2, head.x, head.y,
                          0.0f, 0.0f);

  if (!bot) {
    vp.UpdateBoxNewSectors(ss, head.x, head.y, 0.0f, 0.0f);
  }

  const size_t len = parts.size();
  // as far as having step dist = 42, k = 0.43, sec. size = 300, this could be
  // 300 / 24.0f, with radius 150
  static const size_t tail_step = static_cast<size_t>(WorldConfig::sector_size / tail_step_distance);
  for (size_t i = 3; i < len; i += tail_step) {
    Body &pt = parts[i];
    sbb.UpdateBoxNewSectors(ss, WorldConfig::sector_size / 2, pt.x, pt.y, 0.0f,
                            0.0f);
  }
}

void Snake::UpdateEatenFood(SectorSeq *ss) {
  const uint16_t hx = static_cast<uint16_t>(get_head_x());
  const uint16_t hy = static_cast<uint16_t>(get_head_y());
  const uint16_t r = static_cast<uint16_t>(14 + get_snake_body_part_radius() +
                                           WorldConfig::move_step_distance);
  const int32_t r2 = r * r;

  const uint16_t sx = hx / WorldConfig::sector_size;
  const uint16_t sy = hy / WorldConfig::sector_size;

  // head sector
  {
    Sector *sec = ss->get_sector(sx, sy);
    auto begin = sec->food.begin();
    auto i = sec->FindClosestFood(hx);
    // to left
    {
      auto left = i - 1;
      while (left >= begin && Math::distance_squared(left->x, left->y, hx, hy) <= r2) {
        // std::cout << "eaten food <left> x = " << left->x << ", y = " <<
        // left->y << std::endl;
        on_food_eaten(*left);
        sec->Remove(left);
        i--;
        left--;
      }
    }
    // to right
    {
      auto end = sec->food.end();
      while (i < end && Math::distance_squared(i->x, i->y, hx, hy) <= r2) {
        // std::cout << "eaten food <right> x = " << i->x << ", y = " << i->y <<
        // std::endl;
        on_food_eaten(*i);
        sec->Remove(i);
        end--;
      }
    }
  }
}

BoundBox Snake::get_new_box() const {
  return {{get_head_x(), get_head_y(), 0}, id, this, {}};
}

void Snake::TickAI(long frames) {
  for (auto i = 0; i < frames; i++) {
    // 1. calc angle of radius vector
    float ai_angle = atan2f(get_head_y() - WorldConfig::game_radius,
                            get_head_x() - WorldConfig::game_radius);
    // 2. add pi_2
    ai_angle = Math::normalize_angle(ai_angle + Math::f_pi / 2.0f);
    // 3. set
    if (fabsf(wangle - ai_angle) > 0.01f) {
      wangle = ai_angle;
      update |= change_wangle;
    }
  }
}

bool Snake::Intersect(BoundBoxPos foe, BodySeqCIter prev, BodySeqCIter iter, BodySeqCIter end) const {
  while (iter != end) {
    // weak body part check
    // todo: reduce this whole thing to middle circle check
    if (Math::intersect_circle(iter->x, iter->y, foe.x, foe.y, WorldConfig::move_step_distance * 2)) {
      const float r = foe.r + get_snake_body_part_radius();

      // check actual snake body part
      if (Math::intersect_circle(iter->x, iter->y, foe.x, foe.y, r) ||
          Math::intersect_circle(prev->x, prev->y, foe.x, foe.y, r) ||
          Math::intersect_circle(iter->x + (prev->x - iter->x) / 2.0f,
                                 iter->y + (prev->y - iter->y) / 2.0f, foe.x, foe.y, r)) {
        return true;
      }
    }

    ++prev;
    ++iter;
  }

  return false;
}

bool Snake::Intersect(BoundBoxPos foe) const {
  static const size_t head_size = 8;
  static const size_t tail_step = static_cast<size_t>(WorldConfig::sector_size / tail_step_distance);
  static const size_t tail_step_half = tail_step / 2;
  const size_t len = parts.size();

  if (len <= head_size + tail_step) {
    return Intersect(foe, parts.begin(), parts.begin() + 1, parts.end());
  } else {
    // calculate bb radius, len eval for step dist = 42, k = 0.43
    // parts ..  1  ..  2  ..  3  ..  4  ..  5  ..  6  ..  7  .. tail by 24.0f
    // head center will be i = 3, len [0 .. 3] = 42 * 3 = 126, len [3 .. 7] =
    // 136.9, both < sector_size / 2 = 150
    auto head = parts[3];
    if (Math::intersect_circle(head.x, head.y, foe.x, foe.y, WorldConfig::sector_size / 2)) {
      if (Intersect(foe, parts.begin(), parts.begin() + 1, parts.begin() + 9)) {
        return true;
      }
    }

    // first tail sector center will be... skip 8 + tail_step / 2
    auto end = parts.end();
    for (auto i = parts.begin() + 7 + tail_step_half; i < end; i += tail_step) {
      if (Math::intersect_circle(i->x, i->y, foe.x, foe.y, WorldConfig::sector_size / 2)) {
        auto start = i - tail_step_half;
        auto last = i + tail_step_half;
        if (last > end) {
          last = end;
        }
        if (Intersect(foe, start, start + 1, last)) {
          return true;
        }
      }
    }
  }

  return false;
}

void Snake::on_food_eaten(Food f) {
  IncreaseSnake(f.size);
  eaten.push_back(f);
}

void Snake::IncreaseSnake(uint16_t volume) {
  fullness += volume;
  if (fullness >= 100) {
    fullness -= 100;
    parts.push_back(parts.back());
  }
  update |= change_fullness;
  UpdateSnakeConsts();
}

void Snake::DecreaseSnake(uint16_t volume) {
  if (volume > fullness) {
    volume -= fullness;
    const uint16_t reduce = static_cast<uint16_t>(1 + volume / 100);
    for (uint16_t i = 0; i < reduce; i++) {
      if (parts.size() > 3) {
        const Body &last = parts.back();
        SpawnFood({static_cast<uint16_t>(last.x),
                   static_cast<uint16_t>(last.y),
                   100,  // todo size dep on snake mass, use random
                   skin});
        parts.pop_back();
      }
    }
    fullness = static_cast<uint16_t>(100 - volume % 100);
  } else {
    fullness -= volume;
  }
  update |= change_fullness;
  UpdateSnakeConsts();
}

void Snake::SpawnFood(Food f) {
  const int16_t sx = static_cast<int16_t>(f.x / WorldConfig::sector_size);
  const int16_t sy = static_cast<int16_t>(f.y / WorldConfig::sector_size);

  for (Sector *sec : sbb.sectors) {
    if (sec->x == sx && sec->y == sy) {
      sec->Insert(f);
      spawn.push_back(f);
      break;
    }
  }
}

void Snake::on_dead_food_spawn(SectorSeq *ss, std::function<float()> next_randomf) {
  auto end = parts.end();

  const float r = get_snake_body_part_radius();
  const uint16_t r2 = static_cast<uint16_t>(r * 3);

  const size_t count = static_cast<size_t>(sc * 2);
  const uint8_t food_size = static_cast<uint8_t>(100 / count);

  for (auto i = parts.begin(); i != end; ++i) {
    const uint16_t sx = static_cast<uint16_t>(i->x / WorldConfig::sector_size);
    const uint16_t sy = static_cast<uint16_t>(i->y / WorldConfig::sector_size);
    if (sx > 0 && sx < WorldConfig::sector_count_along_edge - 1 && sy > 0 &&
        sy < WorldConfig::sector_count_along_edge - 1) {
      for (size_t j = 0; j < count; j++) {
        Food f = {static_cast<uint16_t>(i->x + r - next_randomf() * r2),
                  static_cast<uint16_t>(i->y + r - next_randomf() * r2),
                  food_size, static_cast<uint8_t>(29 * next_randomf())};

        Sector *sec = ss->get_sector(sx, sy);
        sec->Insert(f);
        spawn.push_back(f);
      }
    }
  }
}

float Snake::get_snake_scale() const { return gsc; }

float Snake::get_snake_body_part_radius() const { return sbpr; }

std::array<float, WorldConfig::max_snake_parts> get_fmlts() {
  std::array<float, WorldConfig::max_snake_parts> data = {0.0f};
  for (size_t i = 0; i < data.size(); i++) {
    data[i] = powf(1.0f - 1.0f * i / data.size(), 2.25f);
  }
  return data;
}

std::array<float, WorldConfig::max_snake_parts> get_fpsls(
    const std::array<float, WorldConfig::max_snake_parts> &fmlts) {
  std::array<float, WorldConfig::max_snake_parts> data = {0.0f};
  for (size_t i = 1; i < data.size(); i++) {
    data[i] = data[i - 1] + 1.0f / fmlts[i - 1];
  }
  return data;
}

uint16_t Snake::get_snake_score() const {
  static std::array<float, WorldConfig::max_snake_parts> fmlts = get_fmlts();
  static std::array<float, WorldConfig::max_snake_parts> fpsls = get_fpsls(fmlts);

  size_t sct = parts.size() - 1;
  if (sct >= fmlts.size()) {
    sct = fmlts.size() - 1;
  }

  return static_cast<uint16_t>(15.0f * (fpsls[sct] + fullness / 100.0f / fmlts[sct] - 1) - 5);
}

void Snake::UpdateSnakeConsts() {
  gsc = 0.5f + 0.4f / fmaxf(1.0f, 1.0f * (parts.size() - 1 + 16) / 36.0f);
  sc = fminf(6.0f, 1.0f + 1.0f * (parts.size() - 1 - 2) / 106.0f);

  const float scang_x = 1.0f * (7 - parts.size() - 1) / 6.0f;
  scang = 0.13f + 0.87f * scang_x * scang_x;

  ssp = nsp1 + nsp2 * sc;
  fsp = ssp + 0.1f;

  sbpr = 29.0f * 0.5f /* render mode 2 const */ * sc;
}
