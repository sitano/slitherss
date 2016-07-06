#ifndef SLITHER_GAME_WORLD_HPP
#define SLITHER_GAME_WORLD_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "sector.h"
#include "snake.h"

class world {
 public:
  void init(world_config config);
  void init_sectors();
  void init_food();

  void tick(long dt);

  snake::ptr create_snake();
  snake::ptr create_snake_bot();
  void spawn_snakes(const int count);
  void check_snake_bounds(snake* s);

  void init_random();
  int next_random();
  float next_randomf();
  template <typename T>
  T next_random(T base);

  typedef std::unordered_map<snake_id_t, std::shared_ptr<snake>> snakes;
  typedef std::vector<snake_id_t> v_ids;

  void add_snake(snake::ptr ptr);
  void remove_snake(snake_id_t id);
  snakes::iterator get_snake(snake_id_t id);
  snakes& get_snakes();
  sectors& get_sectors();
  v_ids& get_dead();

  std::vector<snake*>& get_changes();

  // before calling this, snake must be flushed()
  void flush_changes(snake_id_t id);
  // before calling this, all snakes must be flushed()
  void flush_changes();

  // const
  static const long virtual_frame_time_ms = 8;

  static const uint8_t protocol_version = 8;

  static constexpr float f_pi = 3.14159265358979323846f;
  static constexpr float f_2pi = 2.0f * f_pi;

 private:
  void tick_snakes(long dt);

 private:
  // todo: reserve to collections
  snakes m_snakes;
  v_ids m_dead;
  sectors m_sectors;
  std::vector<snake*> m_changes;

  // todo fixed point arithmetic
  // todo pools
  // todo sorted checker

  // todo manage overflow, reuse old?
  uint16_t m_lastSnakeId = 0;
  long m_ticks = 0;
  uint32_t m_virtual_frames = 0;

  world_config m_config;
};

std::ostream& operator<<(std::ostream& out, const world& w);

#endif  // SLITHER_GAME_WORLD_HPP
