#ifndef SRC_PACKET_P_PONG_H_
#define SRC_PACKET_P_PONG_H_

#include "packet/p_base.h"

struct packet_pong : public PacketBase {
  packet_pong() : PacketBase(packet_t_pong) {}
  explicit packet_pong(uint16_t clock) : PacketBase(packet_t_pong, clock) {}
};

#endif  // SRC_PACKET_P_PONG_H_
