#ifndef SLITHER_PACKET_PONG_HPP
#define SLITHER_PACKET_PONG_HPP

#include "base.hpp"

struct packet_pong : public packet_base {
    packet_pong() : packet_base(packet_t_pong) {}

    // TODO ...
};

#endif //SLITHER_PACKET_PONG_HPP
