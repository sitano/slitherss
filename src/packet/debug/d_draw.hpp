#ifndef SLITHER_PACKET_DEBUG_DRAW_HPP
#define SLITHER_PACKET_DEBUG_DRAW_HPP

#include "packet/p_base.hpp"

#include <vector>

struct d_draw_point {
    uint16_t x;
    uint16_t y;

    d_draw_point() = default;
    d_draw_point(uint16_t in_x, uint16_t in_y) : x(in_x), y(in_y) { }
    d_draw_point(float in_x, float in_y) : x(static_cast<uint16_t>(in_x)), y(static_cast<uint16_t>(in_y)) { }
};

struct d_draw_dot {
    uint24_t id;
    d_draw_point p;
    uint24_t color;

    static uint8_t get_header() { return '.'; }
    static size_t get_size() { return 1 + 3 + 4 + 3; }
};

struct d_draw_segment {
    uint24_t id;
    d_draw_point v, w;
    uint24_t color;

    static uint8_t get_header() { return '_'; }
    static size_t get_size() { return 1 + 3 + 4 + 4 + 3; }
};

struct d_draw_circle {
    uint24_t id;
    d_draw_point v;
    uint16_t r;
    uint24_t color;

    d_draw_circle() = default;
    d_draw_circle(uint24_t in_id, d_draw_point in_v, uint16_t in_r, uint24_t in_color) : id(in_id), v(in_v), r(in_r), color(in_color) { }
    d_draw_circle(uint24_t in_id, d_draw_point in_v, float in_r, uint24_t in_color) : id(in_id), v(in_v), r(static_cast<uint16_t>(in_r)), color(in_color) { }

    static uint8_t get_header() { return 'o'; }
    static size_t get_size() { return 1 + 3 + 4 + 2 + 3; }
};

struct d_draw_rect {
    uint24_t id;
    d_draw_point v, w;
    uint24_t color;

    static uint8_t get_header() { return '#'; }
    static size_t get_size() { return 1 + 3 + 4 + 4 + 3; }
};

struct packet_debug_draw : public packet_base {
    packet_debug_draw() : packet_base(packet_d_draw) {}

    std::vector<d_draw_dot> dots;
    std::vector<d_draw_segment> segments;
    std::vector<d_draw_rect> rects;
    std::vector<d_draw_circle> circles;

    bool empty() {
        return dots.empty() && segments.empty() && rects.empty() && circles.empty();
    }

    size_t get_size() const noexcept { return 3 +
            dots.size() * d_draw_dot::get_size() +
            segments.size() * d_draw_segment::get_size() +
            rects.size() * d_draw_rect::get_size() +
            circles.size() * d_draw_circle::get_size(); }
};

std::ostream& operator<<(std::ostream & out, const packet_debug_draw & p);

#endif //SLITHER_PACKET_DEBUG_DRAW_HPP
