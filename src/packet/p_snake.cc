#include "packet/p_snake.h"

std::ostream& operator<<(std::ostream& out, const packet_add_snake& p) {
  out << static_cast<PacketBase>(p);

  const Snake* s = p.s;

  out << write_uint16(s->id) << write_ang24(s->angle)  // ehang radians
      << write_uint8(0)                                // unknown
      << write_ang24(s->angle)                         // eangle radians
      << write_fp16<3>(s->speed /
                       32.0f)  // pixels / second -> pixels / 4 * vfr (8ms)
      << write_fp24(s->fullness / 100.0f) << write_uint8(s->skin)
      << write_uint24(s->get_head_x() * 5.0f)
      << write_uint24(s->get_head_y() * 5.0f) << write_string(s->name);

  if (!s->parts.empty()) {
    // from tail to the head
    const auto taili = s->parts.crbegin();
    // last element is a snake position (skip it)
    const auto headi = s->parts.crend() - 1;

    float hx = taili->x;
    float hy = taili->y;
    out << write_uint24(static_cast<uint24_t>(hx * 5.0f))
        << write_uint24(static_cast<uint24_t>(hy * 5.0f));
    for (auto ptr = taili + 1; ptr != headi; ++ptr) {
      const float bpx = ptr->x - hx;
      const float bpy = ptr->y - hy;

      out << write_uint8(static_cast<uint8_t>(bpx * 2.0f + 127.0f))
          << write_uint8(static_cast<uint8_t>(bpy * 2.0f + 127.0f));

      hx += bpx;
      hy += bpy;
    }
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const packet_remove_snake& p) {
  out << static_cast<PacketBase>(p);
  out << write_uint16(p.snakeId);
  out << write_uint8(p.status);
  return out;
}
