#ifndef SLITHER_GAME_SNAKE_HPP
#define SLITHER_GAME_SNAKE_HPP

#include "config.h"
#include "sector.h"

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

enum snake_changes : uint8_t {
    change_pos = 1,
    change_angle = 1 << 1,
    change_wangle = 1 << 2,
    change_speed = 1 << 3,
    change_fullness = 1 << 4,
    change_dying = 1 << 5,
    change_dead = 1 << 6
};

struct body {
    float x;
    float y;

    inline void from(const body& p) {
        x = p.x;
        y = p.y;
    }

    inline void offset(float dx, float dy) {
        x += dx;
        y += dy;
    }

    inline float distance_squared(float dx, float dy) const {
        const float a = x - dx;
        const float b = y - dy;
        return a * a + b * b;
    }
};

struct snake : std::enable_shared_from_this<snake> {
    typedef std::shared_ptr<snake> ptr;

    snake_id_t id;

    uint8_t skin;
    uint8_t update;
    bool acceleration;
    bool bot;

    std::string name;

    // pixels / seconds, base ~185 [px/s]
    uint16_t speed;

    float angle;
    float wangle;

    // 0 - 100, 0 - hungry, 100 - full
    uint16_t fullness;

    snake_bb sbb;
    view_port vp;
    std::vector<body> parts;
    std::vector<food> eaten;
    std::vector<food> spawn;
    size_t clientPartsIndex;

    bool tick(long dt, sectors &ss);
    void tick_ai(long frames);
    void update_box_center();
    void update_box_radius();
    void update_snake_const();
    void init_box_new_sectors(sectors &ss);
    void update_eaten_food(sectors &ss);

    bool intersect(bb_pos foe) const;
    bool intersect(bb_pos foe,
                   std::vector<body>::const_iterator prev,
                   std::vector<body>::const_iterator i,
                   std::vector<body>::const_iterator end) const;

    void eaten_food(food f);
    void increase_snake(uint16_t volume);
    void decrease_snake(uint16_t volume);
    void spawn_food(food f);
    void spawn_food_when_dead(sectors &ss, std::function<float ()> next_randomf);

    float get_snake_scale() const;
    float get_snake_body_part_radius() const;
    uint16_t get_snake_score() const;

    inline const body& get_head() const { return parts[0]; }
    inline float get_head_x() const { return parts[0].x; }
    inline float get_head_y() const { return parts[0].y; }
    inline float get_head_dx() const { return parts[0].x - parts[1].x; }
    inline float get_head_dy() const { return parts[0].y - parts[1].y; }

    std::shared_ptr<snake> get_ptr();
    bb get_new_box() const;

    static constexpr float spangdv = 4.8f;
    static constexpr float nsp1 = 5.39f;
    static constexpr float nsp2 = 0.4f;
    static constexpr float nsp3 = 14.0f;

    static const uint16_t base_move_speed = 185; // pixel in second (convert:  1000*sp/32)
    static const uint16_t boost_speed = 448; // pixel in second (convert:  1000*sp/32)
    static const uint16_t speed_acceleration = 1000; // pixel in second

    static constexpr float prey_angular_speed = 3.5f; // radian in second (convert: 1000ms/8ms * ang[rad])
    static constexpr float snake_angular_speed = 4.125f; // radian in second (convert:  1000ms/8ms * ang[rad])
    static constexpr float snake_tail_k = 0.43f; // snake tail rigidity (0 .. 0.5]

    static const int parts_skip_count = 3;
    static const int parts_start_move_count = 4;
    static constexpr float tail_step_distance = 24.0f; // tail step eval for step dist = 42, k = 0.43
    static constexpr float rot_step_angle = 1.0f * world_config::move_step_distance / boost_speed * snake_angular_speed; // radians step per max acc resolution time
    static const long rot_step_interval = static_cast<long>(1000.0f * rot_step_angle / snake_angular_speed);
    static const long ai_step_interval = 1000;

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

    inline static float normalize_angle(float ang) {
        return ang - f_2pi * floorf( ang / f_2pi );
    }

private:

    float gsc = 0.0f; // snake scale 0.5f + 0.4f / fmaxf(1.0f, 1.0f * (parts.size() - 1 + 16) / 36.0f)
    float sc = 0.0f; // 106th length on snake, min 1, start from 6. Math.min(6, 1 + (f.sct - 2) / 106)
    float scang = 0.0f; // .13 + .87 * Math.pow((7 - f.sc) / 6, 2)
    float ssp = 0.0f; // nsp1 + nsp2 * f.sc;
    float fsp = 0.0f; // f.ssp + .1;
    // snake body part radius, in screen coords it is:
    // - gsc * sbpr * 52 / 32 inner r, and
    // - gsc * sbpr * 62 / 32 for outer r.
    // thus, for sbpr 14.5, inner 21.20, outer 25.28
    float sbpr = 0.0f;

    long m_mov_ticks = 0;
    long m_rot_ticks = 0;
    long m_ai_ticks = 0;
};

#endif //SLITHER_GAME_SNAKE_HPP
