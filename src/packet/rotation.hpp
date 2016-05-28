#ifndef SLITHER_SERVER_ROTATION_HPP
#define SLITHER_SERVER_ROTATION_HPP

#include "base.hpp"
#include "cmath"

struct packet_rotation : public packet_base {
    packet_rotation() : packet_base(packet_t_rot) {}
    packet_rotation(packet_t t) : packet_base(t) {}

    typedef packet_size<8> size;

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    float ang = 0.0f; // 5, int8, ang * pi2 / 256 (current snake angle in radians, clockwise from (1, 0))
    float wang = 0.0f; // 6, int8, wang * pi2 / 256 (target rotation angle snake is heading to)
    float snakeSpeed = 0.0f; // 7, int8, sp / 18 (snake speed?)
};

std::ostream& operator<<(std::ostream & out, const packet_rotation & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);

    if (p.snakeSpeed == 0.0f) {
        out << write_ang8(p.ang);
    } else if (p.wang == 0.0f) {
        out << write_ang8(p.ang);
        out << write_uint8(p.snakeSpeed * 18);
    } else {
        // snake dir = counterclockwise from (1, 0)
        out << write_ang8(p.ang);
        out << write_ang8(p.wang);
        out << write_uint8(p.snakeSpeed * 18);
    }
}

#endif //SLITHER_SERVER_ROTATION_HPP
