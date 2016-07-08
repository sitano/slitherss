#ifndef SRC_PACKET_P_SNAKE_H_
#define SRC_PACKET_P_SNAKE_H_

#include "game/snake.h"
#include "packet/p_base.h"

// Sent when another snake enters range.
struct packet_add_snake : public packet_base {
  packet_add_snake() : packet_base(packet_t_snake), s(nullptr) {}
  explicit packet_add_snake(const snake* input)
      : packet_base(packet_t_snake), s(input) {}

  // 3-4, int16, Snake id
  // 5-7, int24, Snake stop? value * 2*Math.PI / 16777215
  // 8, int8, Unused. The 8. Byte is Unused in the game code. But the Server
  // sends it filled with a value. Maybe we miss something here?
  // 9-11, int24, value * 2*Math.PI / 16777215 snake.eang and snake.wang
  // (Possibly angles of the snake)
  // 12-13, int16, value / 1E3 current speed of snake
  // 14-16, int24, value / 16777215
  // 17, int8, Snake skin (between 9 or 0? and 21)
  // 18-20, int24, value/ 5 snake X pos
  // 21-23, int24, value / 5 snake Y pos
  // 24, int8, Name length
  // 25+Name length, string, Snake nickname
  // ?, int24, Possibly head position (x)
  // ?, int24, Possibly head position (y)
  // ?, int8, Body part position (x)
  // ?, int8, Body part position (y)
  const snake* s;

  size_t get_size() const noexcept {
    return 25 + s->name.length() + 2 * 3 + (s->parts.size() - 1 /* head */) * 2;
  }
};

// Sent when another snake leaves range (that is, close enough to be drawn on
// screen) or dies.
struct packet_remove_snake : public packet_base {
  packet_remove_snake() : packet_base(packet_t_snake) {}
  packet_remove_snake(uint16_t id, uint8_t st)
      : packet_base(packet_t_snake), snakeId(id), status(st) {}

  uint16_t snakeId = 0;  // 3-4, int16, Snake id
  uint8_t status =
      status_snake_left;  // 5, int8, 0 (snake left range) or 1 (snake died)

  size_t get_size() const noexcept { return 6; }

  static const uint8_t status_snake_left = 0;
  static const uint8_t status_snake_died = 1;
};

std::ostream& operator<<(std::ostream& out, const packet_add_snake& p);
std::ostream& operator<<(std::ostream& out, const packet_remove_snake& p);

#endif  // SRC_PACKET_P_SNAKE_H_
