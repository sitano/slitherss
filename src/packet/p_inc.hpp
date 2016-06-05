#ifndef SLITHER_PACKET_INCREMENT_HPP
#define SLITHER_PACKET_INCREMENT_HPP

#include "p_base.hpp"

struct packet_inc : public packet_base {
    packet_inc() : packet_base(packet_t_inc) {}
    explicit packet_inc(out_packet_t t) : packet_base(t) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    uint16_t x = 0; // 5-6, int16, x
    uint16_t y = 0; // 7-8, int16, y
    float fullness = 0.0f; // 9-11, int24, value / 16777215 -> fam

    constexpr size_t get_size() const noexcept { return 12; }
};

struct packet_inc_rel : public packet_inc {
    packet_inc_rel(/* TODO: snake input */) : packet_inc(packet_t_inc_rel) {}

    // 3-4	int16	Snake id
    // 5	int8	value - 128 + head.x -> x
    // 6	int8	value - 128 + head.y -> y
    // 7-9, int24, value / 16777215 -> fam

    constexpr size_t get_size() const noexcept { return 10; }
};

std::ostream& operator<<(std::ostream & out, const packet_inc & p);
std::ostream& operator<<(std::ostream & out, const packet_inc_rel & p);

#endif //SLITHER_PACKET_INCREMENT_HPP
