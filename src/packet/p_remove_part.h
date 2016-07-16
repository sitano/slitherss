#ifndef SRC_PACKET_P_REMOVE_PART_H_
#define SRC_PACKET_P_REMOVE_PART_H_

#include "game/snake.h"
#include "packet/p_base.h"

struct packet_remove_part : public PacketBase {
  packet_remove_part() : PacketBase(packet_t_rem_part) {}
  explicit packet_remove_part(const Snake* s)
      : PacketBase(packet_t_rem_part),
        snakeId(s->id),
        fullness(static_cast<uint8_t>(s->fullness)) {}

  uint16_t snakeId = 0;  // 3-4, int16, Snake id
  uint8_t fullness = 0;  // 5-7, int24, value / 16777215 -> fam

  size_t get_size() const noexcept { return 8; }
};

std::ostream& operator<<(std::ostream& out, const packet_remove_part& p);

#endif  // SRC_PACKET_P_REMOVE_PART_H_
