#include "packet/p_leaderboard.h"

std::ostream& operator<<(std::ostream& out, const packet_leaderboard& p) {
  out << static_cast<PacketBase>(p);
  out << write_uint8(p.leaderboard_rank);
  out << write_uint16(p.local_rank);
  out << write_uint16(p.players);
  for (const auto& ptr : p.top) {
    out << write_uint16(ptr->parts.size());
    out << write_fp24(ptr->fullness / 100.0f);
    out << write_uint8(ptr->skin);
    out << write_string(ptr->name);
  }
  return out;
}
