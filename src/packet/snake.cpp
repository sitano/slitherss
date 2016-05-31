#include "snake.hpp"


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

std::ostream& operator<<(std::ostream & out, const packet_remove_snake & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint8(p.status);
    return out;
}

