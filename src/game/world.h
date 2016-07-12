#ifndef SRC_GAME_WORLD_H_
#define SRC_GAME_WORLD_H_

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>

#include "game/sector.h"
#include "game/snake.h"

typedef std::vector<Snake *> SnakeVec;
typedef std::unordered_map<snake_id_t, std::shared_ptr<Snake>> SnakeMap;
typedef SnakeMap::iterator SnakeMapIter;
typedef std::vector<snake_id_t> Ids;

class World {
 public:
  void Init(WorldConfig in_config);
  void InitSectors();
  void InitFood();

  void Tick(long dt);

  Snake::ptr CreateSnake();
  Snake::ptr CreateSnakeBot();
  void SpawnNumSnakes(const int count);
  void CheckSnakeBounds(Snake *s);

  void InitRandom();
  int NextRandom();
  float NextRandomf();
  template <typename T>
  T NextRandom(T base);

  void AddSnake(Snake::ptr ptr);
  void RemoveSnake(snake_id_t id);
  SnakeMapIter GetSnake(snake_id_t id);
  SnakeMap& GetSnakes();
  sectors& GetSectors();
  Ids& GetDead();

  SnakeVec& GetChangedSnakes();

  // before calling this, snake must be flushed()
  void FlushChanges(snake_id_t id);
  // before calling this, all snakes must be flushed()
  void FlushChanges();

  static const long frame_time_ms = 8;
  static const uint8_t protocol_version = 8;

  static constexpr float f_pi = 3.14159265358979323846f;
  static constexpr float f_2pi = 2.0f * f_pi;

 private:
  void TickSnakes(long dt);

 private:
  // todo: reserve to collections
  SnakeMap snakes;
  Ids dead;
  sectors m_sectors;
  SnakeVec changes;

  // todo pools
  // todo sorted checker

  // todo manage overflow, reuse old?
  uint16_t lastSnakeId = 0;
  long ticks = 0;
  uint32_t frames = 0;

  WorldConfig config;
};

std::ostream& operator<<(std::ostream& out, const World& w);

#endif  // SRC_GAME_WORLD_H_
