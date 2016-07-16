#ifndef SRC_GAME_WORLD_H_
#define SRC_GAME_WORLD_H_

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>

#include "game/sector.h"
#include "game/snake.h"

class World {
 public:
  void Init(WorldConfig in_config);
  void InitSectors();
  void InitFood();

  void Tick(long dt);

  Snake::Ptr CreateSnake();
  Snake::Ptr CreateSnakeBot();
  void SpawnNumSnakes(const int count);
  void CheckSnakeBounds(Snake *s);

  void InitRandom();
  int NextRandom();
  float NextRandomf();
  template <typename T>
  T NextRandom(T base);

  void AddSnake(Snake::Ptr ptr);
  void RemoveSnake(snake_id_t id);
  SnakeMapIter GetSnake(snake_id_t id);
  SnakeMap& GetSnakes();
  SectorSeq& GetSectors();
  Ids& GetDead();

  SnakeVec& GetChangedSnakes();

  // before calling this, snake must be flushed()
  void FlushChanges(snake_id_t id);
  // before calling this, all snakes must be flushed()
  void FlushChanges();

 private:
  void TickSnakes(long dt);

 private:
  // TODO(john.koepi): reserve to collections
  SnakeMap snakes;
  Ids dead;
  SectorSeq sectors;
  SnakeVec changes;

  // TODO(john.koepi) pools
  // TODO(john.koepi) sorted checker

  // TODO(john.koepi) manage overflow, reuse old?
  uint16_t lastSnakeId = 0;
  long ticks = 0;
  uint32_t frames = 0;

  WorldConfig config;
};

std::ostream& operator<<(std::ostream& out, const World& w);

#endif  // SRC_GAME_WORLD_H_
