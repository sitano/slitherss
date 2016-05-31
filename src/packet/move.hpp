#ifndef SLITHER_PACKET_MOVE_HPP
#define SLITHER_PACKET_MOVE_HPP

#include "base.hpp"

struct packet_move : public packet_base {
    packet_move() : packet_base(packet_t_mov) {}
    packet_move(packet_t t) : packet_base(t) {}

    uint16_t snakeId; // 3-4, int16, Snake id
    uint16_t x; // 5-6, int16, x
    uint16_t y; // 7-8, int16, y

    size_t get_size() { return 9; }
};

struct packet_move_rel : public packet_move {
    packet_move_rel(/* TODO: snake input */) : packet_move(packet_t_mov_rel) {}

    // 3-4	int16	Snake id
    // 5	int8	value - 128 + head.x -> x
    // 6	int8	value - 128 + head.y -> y

    size_t get_size() { return 7; }
};

std::ostream& operator<<(std::ostream & out, const packet_move & p);
std::ostream& operator<<(std::ostream & out, const packet_move_rel & p);

#endif //SLITHER_PACKET_MOVE_HPP
