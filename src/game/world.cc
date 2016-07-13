#include "game/world.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <vector>

Snake::Ptr World::CreateSnake() {
  lastSnakeId++;

  auto s = std::make_shared<Snake>();
  s->id = lastSnakeId;
  s->name = "";
  s->skin = static_cast<uint8_t>(9 + NextRandom(21 - 9 + 1));
  s->speed = Snake::base_move_speed;
  s->fullness = 0;

  float angle = World::f_2pi * NextRandomf();
  float dist = 1000.0f + NextRandom(5000);
  uint16_t x = static_cast<uint16_t>(WorldConfig::game_radius + dist * cosf(angle));
  uint16_t y = static_cast<uint16_t>(WorldConfig::game_radius + dist * sinf(angle));
  angle = Snake::normalize_angle(angle + f_pi);
  // const uint16_t half_radius = game_radius / 2;
  // uint16_t x = game_radius + NextRandom(game_radius) - half_radius;
  // uint16_t y = game_radius + NextRandom(game_radius) - half_radius;
  // todo: reserve snake.parts at least for sizeof(snake) bytes
  const int len = 1 /* head */ + 2 /* body min = 2 */ +
    std::max(config.snake_min_length,
    NextRandom(config.snake_average_length));

  for (int i = 0; i < len && i < Snake::parts_skip_count + Snake::parts_start_move_count; ++i) {
    s->parts.push_back(Body{1.0f * x, 1.0f * y});
    x += cosf(angle) * WorldConfig::move_step_distance;
    y += sinf(angle) * WorldConfig::move_step_distance;
  }

  for (int i = Snake::parts_skip_count + Snake::parts_start_move_count; i < len; ++i) {
    s->parts.push_back(Body{1.0f * x, 1.0f * y});
    x += cosf(angle) * Snake::tail_step_distance;
    y += sinf(angle) * Snake::tail_step_distance;
  }

  s->clientPartsIndex = s->parts.size();
  s->angle = Snake::normalize_angle(angle + f_pi);
  s->wangle = Snake::normalize_angle(angle + f_pi);

  s->sbb = SnakeBoundBox(s->get_new_box());
  s->vp = ViewPort(s->get_new_box());
  s->UpdateBoxCenter();
  s->UpdateBoxRadius();
  s->UpdateSnakeConsts();
  s->InitBoxNewSectors(&sectors);

  return s;
}

Snake::Ptr World::CreateSnakeBot() {
  Snake::Ptr ptr = CreateSnake();
  ptr->bot = true;
  return ptr;
}

void World::InitRandom() { std::srand(std::time(nullptr)); }

int World::NextRandom() { return std::rand(); }

float World::NextRandomf() { return 1.0f * std::rand() / RAND_MAX; }

template <typename T>
T World::NextRandom(T base) {
  return static_cast<T>(NextRandom() % base);
}

void World::Tick(long dt) {
  ticks += dt;
  const long vfr = ticks / WorldConfig::frame_time_ms;
  if (vfr > 0) {
    const long vfr_time = vfr * WorldConfig::frame_time_ms;
    TickSnakes(vfr_time);

    ticks -= vfr_time;
    frames += vfr;
  }
}

void World::TickSnakes(long dt) {
  for (auto pair : snakes) {
    Snake *const s = pair.second.get();

    if (s->Tick(dt, &sectors)) {
      changes.push_back(s);
    }
  }

  for (auto s : changes) {
    if (s->update & change_pos) {
      CheckSnakeBounds(s);
    }
  }
}

void World::CheckSnakeBounds(Snake *s) {
  static std::vector<snake_id_t> cs_cache;
  cs_cache.clear();

  // world bounds
  const Body &head = s->get_head();
  if (head.DistanceSquared(WorldConfig::game_radius, WorldConfig::game_radius) >=
      WorldConfig::death_radius * WorldConfig::death_radius) {
    s->update |= change_dying;
    return;
  }

  // because we check after move being made
  BoundBoxPos check(s->get_head_x(), s->get_head_y(),
               s->get_snake_body_part_radius());

  // check bound coverage
  const int16_t sx = static_cast<int16_t>(check.x / WorldConfig::sector_size);
  const int16_t sy = static_cast<int16_t>(check.y / WorldConfig::sector_size);
  static const int16_t width = 1;

  // 3x3 check head coverage
  static const int16_t map_width_sectors =
      static_cast<int16_t>(WorldConfig::sector_count_along_edge);
  for (int16_t j = sy - width; j <= sy + width; j++) {
    for (int16_t i = sx - width; i <= sx + width; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        Sector *sec_ptr = sectors.get_sector(i, j);
        // check sector intersects head
        if (sec_ptr->Intersect(
            {check.x, check.y, WorldConfig::move_step_distance})) {
          // check sector snakes
          for (const BoundBox *bb_ptr : sec_ptr->snakes) {
            const Snake *s2 = bb_ptr->snake;
            if (s == s2) {
              continue;
            }

            // check if snakes already checked
            if (std::find(cs_cache.begin(), cs_cache.end(), s2->id) != cs_cache.end()) {
              continue;
            } else {
              cs_cache.push_back(s2->id);
            }

            if (s2->Intersect(check)) {
              s->update |= change_dying;
              return;
            }
          }
        }
      }
    }
  }

  // std::cout << "intersects " << i << ", SectorSeq " <<
  // s->box.get_sectors_count() << ", snakes/in/s " <<
  // s->box.get_snakes_in_sectors_count() << std::endl;
}

void World::Init(WorldConfig in_config) {
  config = in_config;

  InitRandom();
  InitSectors();
  InitFood();

  SpawnNumSnakes(in_config.bots);
}

void World::InitSectors() {
  sectors.InitSectors();
}

void World::InitFood() {
  for (Sector &s : sectors) {
    const uint8_t cx = WorldConfig::sector_count_along_edge / 2;
    const uint8_t cy = cx;
    const uint16_t dist = (s.x - cx) * (s.x - cx) + (s.y - cy) * (s.y - cy);
    const float dp = 1.0f -
                     1.0f * dist / (WorldConfig::sector_count_along_edge *
                                    WorldConfig::sector_count_along_edge);
    const size_t density = static_cast<size_t>(dp * 10);
    for (size_t i = 0; i < density; i++) {
      s.food.push_back(
          Food{static_cast<uint16_t>(
                   s.x * WorldConfig::sector_size +
                       NextRandom<uint16_t>(WorldConfig::sector_size)),
               static_cast<uint16_t>(
                   s.y * WorldConfig::sector_size +
                       NextRandom<uint16_t>(WorldConfig::sector_size)),
               static_cast<uint8_t>(1 + NextRandom<uint8_t>(10)),
               NextRandom<uint8_t>(29)});
    }
    s.Sort();
  }
}

void World::AddSnake(Snake::Ptr ptr) {
  snakes.insert({ptr->id, ptr});
}

void World::RemoveSnake(snake_id_t id) {
  FlushChanges(id);

  auto sn_i = GetSnake(id);
  if (sn_i != snakes.end()) {
    for (auto sec_ptr : sn_i->second->sbb.sectors) {
      sec_ptr->RemoveSnake(id);
    }

    snakes.erase(id);
  }
}

SnakeMapIter World::GetSnake(snake_id_t id) {
  return snakes.find(id);
}

SnakeMap &World::GetSnakes() { return snakes; }

Ids &World::GetDead() { return dead; }

std::vector<Snake *> &World::GetChangedSnakes() { return changes; }

void World::FlushChanges() { changes.clear(); }

void World::FlushChanges(snake_id_t id) {
  changes.erase(
      std::remove_if(changes.begin(), changes.end(),
                     [id](const Snake *s) { return s->id == id; }), changes.end());
}

void World::SpawnNumSnakes(const int count) {
  for (int i = 0; i < count; i++) {
    AddSnake(CreateSnakeBot());
  }
}

SectorSeq &World::GetSectors() { return sectors; }

std::ostream &operator<<(std::ostream &out, const World &w) {
  return out << "\tgame_radius = " << WorldConfig::game_radius
             << "\n\tmax_snake_parts = " << WorldConfig::max_snake_parts
             << "\n\tsector_size = " << WorldConfig::sector_size
             << "\n\tsector_count_along_edge = " << WorldConfig::sector_count_along_edge
             << "\n\tvirtual_frame_time_ms = " << WorldConfig::frame_time_ms
             << "\n\tprotocol_version = " << static_cast<long>(WorldConfig::protocol_version)
             << "\n\tspangdv = " << Snake::spangdv
             << "\n\tnsp1 = " << Snake::nsp1 << "\n\tnsp2 = " << Snake::nsp2
             << "\n\tnsp3 = " << Snake::nsp3
             << "\n\tbase_move_speed = " << Snake::base_move_speed
             << "\n\tboost_speed = " << Snake::boost_speed
             << "\n\tspeed_acceleration = " << Snake::speed_acceleration
             << "\n\tprey_angular_speed = " << Snake::prey_angular_speed
             << "\n\tsnake_angular_speed = " << Snake::snake_angular_speed
             << "\n\tsnake_tail_k = " << Snake::snake_tail_k
             << "\n\tparts_skip_count = " << Snake::parts_skip_count
             << "\n\tparts_start_move_count = " << Snake::parts_start_move_count
             << "\n\tmove_step_distance = " << WorldConfig::move_step_distance
             << "\n\trot_step_angle = " << Snake::rot_step_angle;
}
