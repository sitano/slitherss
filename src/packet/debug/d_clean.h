#ifndef SRC_PACKET_DEBUG_D_CLEAN_H_
#define SRC_PACKET_DEBUG_D_CLEAN_H_

#include "packet/p_base.h"

// reset debug render buffer
struct packet_debug_reset : public PacketBase {
    packet_debug_reset() : PacketBase(packet_d_reset) {}

    size_t get_size() const noexcept { return 3; }
};

std::ostream& operator<<(std::ostream & out, const packet_debug_reset & p);

#endif  // SRC_PACKET_DEBUG_D_CLEAN_H_
