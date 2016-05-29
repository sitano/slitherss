#ifndef SLITHER_PACKET_SNAKE_HPP
#define SLITHER_PACKET_SNAKE_HPP

#include "base.hpp"
#include "game/snake.hpp"

#include <memory>

// Sent when another snake enters range.
struct packet_add_snake : public packet_base {
    packet_add_snake() : packet_base(packet_t_snake) {}

    // 3-4, int16, Snake id
    // 5-7, int24, Snake stop? value * 2*Math.PI / 16777215
    // 8, int8, Unused. The 8. Byte is Unused in the game code. But the Server sends it filled with a value. Maybe we miss something here?
    // 9-11, int24, value * 2*Math.PI / 16777215 snake.eang and snake.wang (Possibly angles of the snake)
    // 12-13, int16, value / 1E3 current speed of snake
    // 14-16, int24, value / 16777215
    // 17, int8, Snake skin (between 9 or 0? and 21)
    // 18-20, int24, value/ 5 snake X pos
    // 21-23, int24, value / 5 snake Y pos
    // 24, int8, Name length
    // 25+Name length, string, Snake nickname
    // ?, int24, Possibly head position (x)
    // ?, int24, Possibly head position (y)
    // ?, int8, Body part position (x)
    // ?, int8, Body part position (y)
    std::shared_ptr<snake> s;

    size_t get_size() {
        return 25 + 1 + s->name.length() + 2 * 3 + (s->parts.size() - 1 /* head */) * 2;
    }
};

std::ostream& operator<<(std::ostream & out, const packet_add_snake & p) {
    out << static_cast<packet_base>(p);

    const snake & s= p.s.operator*();

    out << write_uint16(s.id)
        << write_fp24(s.ehang)
        << write_uint8(0) // unknown
        << write_fp24(s.eangle)
        << write_fp16<3>(s.speed)
        << write_fp24(s.fullness)
        << write_uint8(s.color)
        << write_uint24(s.x * 5)
        << write_uint24(s.y * 5)
        << write_string(s.name);

    if (!s.parts.empty()) {
        const body &head = s.parts.front();
        uint32_t hx = head.x;
        uint32_t hy = head.y;
        out << write_uint24(hx * 5) << write_uint24(hy * 5);
        for (auto ptr = (++ s.parts.cbegin()); ptr != s.parts.cend(); ++ ptr) {
            const int32_t bpx = ptr->x - hx;
            const int32_t bpy = ptr->y - hy;

            out << write_uint8((bpx + 127) * 2)
                << write_uint8((bpy + 127) * 2);

            hx += bpx;
            hy += bpy;
        }
    }

    return out;
}

// Sent when another snake leaves range (that is, close enough to be drawn on screen) or dies.
struct packet_remove_snake : public packet_base {
    packet_remove_snake() : packet_base(packet_t_snake) {}

    uint16_t snakeId; // 3-4, int16, Snake id
    uint8_t status; // 5, int8, 0 (snake left range) or 1 (snake died)

    size_t get_size() { return 6; }

    static const uint8_t status_snake_left = 0;
    static const uint8_t status_snake_died = 1;
};

std::ostream& operator<<(std::ostream & out, const packet_remove_snake & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint8(p.status);
    return out;
}

#endif //SLITHER_PACKET_SNAKE_HPP
