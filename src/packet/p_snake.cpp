#include "p_snake.hpp"

std::ostream& operator<<(std::ostream & out, const packet_add_snake & p) {
    out << static_cast<packet_base>(p);

    const snake & s= p.s.operator*();

    out << write_uint16(s.id)
        << write_fp24(s.angle) // ehang radians
        << write_uint8(0) // unknown
        << write_fp24(s.angle) // eangle radians
        << write_fp16<3>(s.speed / 32.0f) // pixels / second -> pixels / 4 * vfr (8ms)
        << write_fp24(s.fullness / 100.0f)
        << write_uint8(s.skin)
        << write_uint24(s.get_head_x() * 5.0f)
        << write_uint24(s.get_head_y() * 5.0f)
        << write_string(s.name);

    if (!s.parts.empty()) {
        const body &head = s.parts.front();
        float hx = head.x;
        float hy = head.y;
        out << write_uint24(static_cast<uint24_t>(hx * 5.0f))
            << write_uint24(static_cast<uint24_t>(hy * 5.0f));
        for (auto ptr = (++ s.parts.cbegin()); ptr != s.parts.cend(); ++ ptr) {
            const float bpx = ptr->x - hx;
            const float bpy = ptr->y - hy;

            out << write_uint8(static_cast<uint8_t>((bpx + 127.0f) * 2.0f))
                << write_uint8(static_cast<uint8_t>((bpy + 127.0f) * 2.0f));

            hx += bpx;
            hy += bpy;
        }
    }

    return out;
}

std::ostream& operator<<(std::ostream & out, const packet_remove_snake & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint8(p.status);
    return out;
}

