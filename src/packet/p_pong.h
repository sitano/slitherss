#ifndef SLITHER_PACKET_PONG_HPP
#define SLITHER_PACKET_PONG_HPP

#include "p_base.h"

struct packet_pong : public packet_base {
  packet_pong() : packet_base(packet_t_pong) {}
  explicit packet_pong(uint16_t clock) : packet_base(packet_t_pong, clock) {}
};

#endif  // SLITHER_PACKET_PONG_HPP
