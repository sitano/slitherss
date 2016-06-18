#ifndef SLITHER_PACKET_DEBUG_CLEAN_HPP
#define SLITHER_PACKET_DEBUG_CLEAN_HPP

#include "packet/p_base.hpp"

// reset debug render buffer
struct packet_debug_reset : public packet_base {
    explicit packet_debug_reset() : packet_base(packet_d_reset) {}

    size_t get_size() const noexcept { return 3; }
};

std::ostream& operator<<(std::ostream & out, const packet_debug_reset & p);

#endif //SLITHER_PACKET_DEBUG_CLEAN_HPP
