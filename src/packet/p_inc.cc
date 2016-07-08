#include "packet/p_inc.h"

std::ostream& operator<<(std::ostream& out, const packet_inc& p) {
  out << static_cast<packet_base>(p);
  out << write_uint16(p.snakeId);
  out << write_uint16(p.x);
  out << write_uint16(p.y);
  out << write_fp24(p.fullness / 100.0f);
  return out;
}

std::ostream& operator<<(std::ostream& out, const packet_inc_rel& p) {
  out << static_cast<packet_base>(p);
  out << write_uint16(p.snakeId);
  out << write_uint8(p.dx);
  out << write_uint8(p.dy);
  out << write_fp24(p.fullness / 100.0f);
  return out;
}
