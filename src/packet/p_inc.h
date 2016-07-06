#ifndef SLITHER_PACKET_INCREMENT_HPP
#define SLITHER_PACKET_INCREMENT_HPP

#include "p_base.h"
#include "game/snake.h"

struct packet_inc : public packet_base {
    packet_inc() : packet_base(packet_t_inc) {}
    packet_inc(uint16_t in_snakeId, uint16_t in_x, uint16_t in_y, uint8_t in_f) :
            packet_base(packet_t_inc), snakeId(in_snakeId), x(in_x), y(in_y), fullness(in_f) {}
    explicit packet_inc(const snake *s): packet_base(packet_t_inc),
                           snakeId(s->id),
                           x(static_cast<uint16_t>(s->get_head_x())),
                           y(static_cast<uint16_t>(s->get_head_y())),
                           fullness(static_cast<uint8_t>(s->fullness)) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    uint16_t x = 0; // 5-6, int16, x
    uint16_t y = 0; // 7-8, int16, y
    uint8_t fullness = 0; // 9-11, int24, value / 16777215 -> fam

    size_t get_size() const noexcept { return 12; }
};

struct packet_inc_rel : public packet_base {
    packet_inc_rel(/* TODO: snake input */) : packet_base(packet_t_inc_rel) {}
    packet_inc_rel(uint16_t in_snakeId, uint8_t in_dx, uint8_t in_dy, uint8_t in_f) :
            packet_base(packet_t_inc_rel), snakeId(in_snakeId), dx(in_dx), dy(in_dy), fullness(in_f) {}
    explicit packet_inc_rel(const snake *s): packet_base(packet_t_inc_rel),

                           snakeId(s->id),
                           dx(static_cast<uint8_t>(s->get_head_dx())),
                           dy(static_cast<uint8_t>(s->get_head_dy())),
                           fullness(static_cast<uint8_t>(s->fullness)) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    uint8_t dx = 0; // 5	int8	value - 128 + head.x -> x
    uint8_t dy = 0; // 6	int8	value - 128 + head.y -> y
    uint8_t fullness = 0; // 9-11, int24, value / 16777215 -> fam

    size_t get_size() const noexcept { return 10; }
};

std::ostream& operator<<(std::ostream & out, const packet_inc & p);
std::ostream& operator<<(std::ostream & out, const packet_inc_rel & p);

#endif //SLITHER_PACKET_INCREMENT_HPP
