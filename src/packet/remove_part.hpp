#ifndef SLITHER_SERVER_REMOVE_PART_HPP
#define SLITHER_SERVER_REMOVE_PART_HPP

#include "base.hpp"

struct packet_remove_part : public packet_base {
    packet_remove_part() : packet_base(packet_t_rem_part) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    float fullness = 0.0f; // 5-7, int24, value / 16777215 -> fam

    size_t get_size() { return 8; }
};

std::ostream& operator<<(std::ostream & out, const packet_remove_part & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    if (p.fullness != 0.0f) {
        out << write_fp24(p.fullness);
    }
    return out;
}

#endif //SLITHER_SERVER_REMOVE_PART_HPP
