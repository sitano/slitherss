#ifndef SLITHER_PACKET_REMOVE_PART_HPP
#define SLITHER_PACKET_REMOVE_PART_HPP

#include "game/snake.h"
#include "p_base.h"

struct packet_remove_part : public packet_base {
  packet_remove_part() : packet_base(packet_t_rem_part) {}
  explicit packet_remove_part(const snake* s)
      : packet_base(packet_t_rem_part),
        snakeId(s->id),
        fullness(static_cast<uint8_t>(s->fullness)) {}

  uint16_t snakeId = 0;  // 3-4, int16, Snake id
  uint8_t fullness = 0;  // 5-7, int24, value / 16777215 -> fam

  size_t get_size() const noexcept { return 8; }
};

std::ostream& operator<<(std::ostream& out, const packet_remove_part& p);

#endif  // SLITHER_PACKET_REMOVE_PART_HPP
