#ifndef SLITHER_SERVER_INCREMENT_HPP
#define SLITHER_SERVER_INCREMENT_HPP

#include "base.hpp"

struct packet_inc : public packet_base {
    packet_inc() : packet_base(packet_t_inc) {}
    packet_inc(packet_t t) : packet_base(t) {}

    uint16_t snakeId; // 3-4, int16, Snake id
    uint16_t x; // 5-6, int16, x
    uint16_t y; // 7-8, int16, y
    float fullness = 0.0f; // 9-11, int24, value / 16777215 -> fam

    size_t get_size() { return 12; }
};

struct packet_inc_rel : public packet_inc {
    packet_inc_rel(/* TODO: snake input */) : packet_inc(packet_t_inc_rel) {}

    // 3-4	int16	Snake id
    // 5	int8	value - 128 + head.x -> x
    // 6	int8	value - 128 + head.y -> y
    float fullness = 0.0f; // 7-9, int24, value / 16777215 -> fam

    size_t get_size() { return 10; }
};

std::ostream& operator<<(std::ostream & out, const packet_inc & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint16(p.x);
    out << write_uint16(p.y);
    out << write_fp24(p.fullness);
    return out;
}

std::ostream& operator<<(std::ostream & out, const packet_inc_rel & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint8(p.x);
    out << write_uint8(p.y);
    out << write_fp24(p.fullness);
    return out;
}

#endif //SLITHER_SERVER_INCREMENT_HPP
