#include "packet/p_init.h"

std::ostream& operator<<(std::ostream& out, const packet_init& p) {
  return out << static_cast<packet_base>(p) << write_uint24(p.game_radius)
             << write_uint16(p.max_snake_parts) << write_uint16(p.sector_size)
             << write_uint16(p.sector_count_along_edge) << write_fp8(p.spangdv)
             << write_fp16<2>(p.nsp1) << write_fp16<2>(p.nsp2)
             << write_fp16<2>(p.nsp3) << write_fp16<3>(p.snake_ang_speed)
             << write_fp16<3>(p.prey_ang_speed) << write_fp16<3>(p.snake_tail_k)
             << write_uint8(p.protocol_version);
}
