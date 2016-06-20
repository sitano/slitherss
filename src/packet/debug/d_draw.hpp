#ifndef SLITHER_PACKET_DEBUG_DRAW_HPP
#define SLITHER_PACKET_DEBUG_DRAW_HPP

#include "packet/p_base.hpp"

#include <vector>

struct d_draw_dot {
    uint24_t id;
    uint16_t x;
    uint16_t y;

    d_draw_dot() = default;
    d_draw_dot(uint16_t in_x, uint16_t in_y) : id(0), x(in_x), y(in_y) {}
    d_draw_dot(uint16_t in_id, uint16_t in_x, uint16_t in_y) : id(in_id), x(in_x), y(in_y) {}

    static uint8_t get_header() { return '.'; }
    static size_t get_size() { return 1 + 3 + 2 + 2; }
};

struct d_draw_segment {
    uint24_t id;
    d_draw_dot v, w;
    uint8_t color;

    static uint8_t get_header() { return '_'; }
    static size_t get_size() { return 1 + 3 + 4 + 4 + 1; }
};

struct d_draw_circle {
    uint24_t id;
    d_draw_dot v;
    uint16_t r; // squared
    uint8_t color;

    static uint8_t get_header() { return 'o'; }
    static size_t get_size() { return 1 + 3 + 4 + 2 + 1; }
};

struct packet_debug_draw : public packet_base {
    packet_debug_draw() : packet_base(packet_d_draw) {}

    std::vector<d_draw_dot> dots;
    std::vector<d_draw_segment> segments;
    std::vector<d_draw_circle> circles;

    bool empty() {
        return dots.empty() && segments.empty() && circles.empty();
    }

    size_t get_size() const noexcept { return 3 +
            dots.size() * d_draw_dot::get_size() +
            segments.size() * d_draw_segment::get_size() +
            circles.size() * d_draw_circle::get_size(); }
};

std::ostream& operator<<(std::ostream & out, const packet_debug_draw & p);

#endif //SLITHER_PACKET_DEBUG_DRAW_HPP
