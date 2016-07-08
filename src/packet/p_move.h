#ifndef SRC_PACKET_P_MOVE_H_
#define SRC_PACKET_P_MOVE_H_

#include "game/snake.h"
#include "packet/p_base.h"

struct packet_move : public packet_base {
  packet_move() : packet_base(packet_t_mov) {}
  packet_move(uint16_t in_snakeId, uint16_t in_x, uint16_t in_y)
      : packet_base(packet_t_mov), snakeId(in_snakeId), x(in_x), y(in_y) {}
  explicit packet_move(const snake* s)
      : packet_base(packet_t_mov),
        snakeId(s->id),
        x(static_cast<uint16_t>(s->get_head_x())),
        y(static_cast<uint16_t>(s->get_head_y())) {}

  uint16_t snakeId = 0;  // 3-4, int16, Snake id
  uint16_t x = 0;        // 5-6, int16, x
  uint16_t y = 0;        // 7-8, int16, y

  size_t get_size() const noexcept { return 9; }
};

struct packet_move_rel : public packet_base {
  packet_move_rel() : packet_base(packet_t_mov_rel) {}

  packet_move_rel(uint16_t _snakeId, int8_t _dx, int8_t _dy)
      : packet_base(packet_t_mov_rel),
        snakeId(_snakeId),
        dx(static_cast<uint8_t>(_dx + 128)),
        dy(static_cast<uint8_t>(_dy + 128)) {}

  explicit packet_move_rel(const snake* s)
      : packet_base(packet_t_mov),
        snakeId(s->id),
        dx(static_cast<uint8_t>(s->get_head_dx())),
        dy(static_cast<uint8_t>(s->get_head_dy())) {}

  uint16_t snakeId = 0;  // 3-4  int16   Snake id
  uint8_t dx = 0;        // 5    int8    value - 128 + head.x -> x
  uint8_t dy = 0;        // 6    int8    value - 128 + head.y -> y

  size_t get_size() const noexcept { return 7; }
};

std::ostream& operator<<(std::ostream& out, const packet_move& p);
std::ostream& operator<<(std::ostream& out, const packet_move_rel& p);

#endif  // SRC_PACKET_P_MOVE_H_
