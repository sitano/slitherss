#ifndef SLITHER_PACKET_ROTATION_HPP
#define SLITHER_PACKET_ROTATION_HPP

#include "p_base.hpp"
#include <cmath>

struct packet_rotation : public packet_base {
    packet_rotation() = default;
    explicit packet_rotation(out_packet_t t) : packet_base(t) {}

    uint16_t snakeId = 0; // 3-4, int16, Snake id
    float ang = -1.0f; // 5, int8, ang * pi2 / 256 (current snake angle in radians, clockwise from (1, 0))
    float wang = -1.0f; // 6, int8, wang * pi2 / 256 (target rotation angle snake is heading to)
    float snakeSpeed = -1.0f; // 7, int8, sp / 18 (snake speed?)

    size_t get_size() const noexcept { return 8; }
    bool is_clockwise() const noexcept;

    out_packet_t get_rot_type() const noexcept;

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

    inline static float normalize_angle(float angle) {
        return angle - f_2pi * floorf( angle / f_2pi );
    }
};

std::ostream& operator<<(std::ostream & out, const packet_rotation & p);

#endif //SLITHER_PACKET_ROTATION_HPP
