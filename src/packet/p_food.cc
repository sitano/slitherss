#include "packet/p_food.h"

std::ostream& operator<<(std::ostream& out, const packet_set_food& p) {
  out << static_cast<packet_base>(p);
  for (const food& f : *p.food_ptr) {
    out << write_uint8(f.color) << write_uint16(f.x) << write_uint16(f.y)
        << write_uint8(f.size * 5);
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const packet_spawn_food& p) {
  out << static_cast<packet_base>(p) << write_uint8(p.m_food.color)
      << write_uint16(p.m_food.x) << write_uint16(p.m_food.y)
      << write_uint8(p.m_food.size * 5);
  return out;
}

std::ostream& operator<<(std::ostream& out, const packet_add_food& p) {
  out << static_cast<packet_base>(p) << write_uint8(p.m_food.color)
      << write_uint16(p.m_food.x) << write_uint16(p.m_food.y)
      << write_uint8(p.m_food.size * 5);
  return out;
}

std::ostream& operator<<(std::ostream& out, const packet_eat_food& p) {
  out << static_cast<packet_base>(p) << write_uint16(p.m_food.x)
      << write_uint16(p.m_food.y);
  if (p.snakeId > 0) {
    out << write_uint16(p.snakeId);
  }
  return out;
}
