#ifndef SLITHER_SERVER_ROTATION_HPP
#define SLITHER_SERVER_ROTATION_HPP

#include "base.hpp"

struct packet_fullness : public packet_base {
    packet_fullness() : packet_base(packet_t_set_fullness) {}

    typedef packet_size<8> size;

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    float fullness = 0.0f; // 5-7, int24, value / 16777215 -> fam
};

std::ostream& operator<<(std::ostream & out, const packet_fullness & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_fp24(p.fullness);
    return out;
}

#endif //SLITHER_SERVER_ROTATION_HPP
