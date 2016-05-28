#ifndef SLITHER_SERVER_ROTATION_HPP
#define SLITHER_SERVER_ROTATION_HPP

#include "base.hpp"

struct packet_move : public packet_base {
    packet_move() : packet_base(packet_t_move) {}
    packet_move(packet_t t) : packet_base(t) {}

    typedef packet_size<9> size;

    uint16_t snakeId; // 3-4, int16, Snake id
    uint16_t x; // 5-6, int16, x
    uint16_t y; // 7-8, int16, y
};

struct packet_move_rel : public packet_move {
    packet_move_rel(/* TODO: snake input */) : packet_move_rel(packet_t_mov_rel) {}

    // 3-4	int16	Snake id
    // 5	int8	value - 128 + head.x -> x
    // 6	int8	value - 128 + head.y -> y

    typedef packet_size<7> size;
};

std::ostream& operator<<(std::ostream & out, const packet_move & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint16(p.x);
    out << write_uint16(p.y);
    return out;
}

std::ostream& operator<<(std::ostream & out, const packet_move_rel & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint8(p.x);
    out << write_uint8(p.y);
    return out;
}

#endif //SLITHER_SERVER_ROTATION_HPP
