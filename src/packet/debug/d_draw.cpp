#include "d_draw.hpp"

std::ostream& operator<<(std::ostream & out, const packet_debug_draw & p) {
    out << static_cast<packet_base>(p);

    for (const d_draw_dot &v : p.dots) {
        out << write_uint8(d_draw_dot::get_header())
            << write_uint24(v.id)
            << write_uint16(v.x)
            << write_uint16(v.y);
    }

    for (const d_draw_segment &v : p.segments) {
        out << write_uint8(d_draw_segment::get_header())
            << write_uint24(v.id)
            << write_uint16(v.v.x)
            << write_uint16(v.v.y)
            << write_uint16(v.w.x)
            << write_uint16(v.w.y)
            << write_uint8(v.color);
    }

    for (const d_draw_circle &v : p.circles) {
        out << write_uint8(d_draw_circle::get_header())
            << write_uint24(v.id)
            << write_uint16(v.v.x)
            << write_uint16(v.v.y)
            << write_uint24(v.r2)
            << write_uint8(v.color);
    }

    return out;
}

