#ifndef SLITHER_PACKET_INCREMENT_HPP
#define SLITHER_PACKET_INCREMENT_HPP

#include "p_base.hpp"

struct packet_inc : public packet_base {
    packet_inc() : packet_base(packet_t_inc) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    uint16_t x = 0; // 5-6, int16, x
    uint16_t y = 0; // 7-8, int16, y
    uint8_t fullness = 0; // 9-11, int24, value / 16777215 -> fam

    constexpr size_t get_size() const noexcept { return 12; }
};

struct packet_inc_rel : public packet_base {
    packet_inc_rel(/* TODO: snake input */) : packet_base(packet_t_inc_rel) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    uint8_t dx = 0; // 5	int8	value - 128 + head.x -> x
    uint8_t dy = 0; // 6	int8	value - 128 + head.y -> y
    uint8_t fullness = 0; // 9-11, int24, value / 16777215 -> fam

    constexpr size_t get_size() const noexcept { return 10; }
};

std::ostream& operator<<(std::ostream & out, const packet_inc & p);
std::ostream& operator<<(std::ostream & out, const packet_inc_rel & p);

#endif //SLITHER_PACKET_INCREMENT_HPP
