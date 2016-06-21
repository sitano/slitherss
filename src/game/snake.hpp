#ifndef SLITHER_GAME_SNAKE_HPP
#define SLITHER_GAME_SNAKE_HPP

#include "config.hpp"
#include "sector.hpp"

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
    change_dying = 1 << 4,
    change_dead = 1 << 5
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
    uint8_t fullness;

    snake_bb bb;
    snake_bb vp;
    std::vector<body> parts;

    bool tick(long dt, sectors &ss);
    void tick_ai(long frames);
    void update_box_center();
    void update_box_radius();
    void init_box_new_sectors(sectors &ss);
    template <int sr, bool track>
    void update_box_new_sectors(sectors &ss, snake_bb *box, const float new_x, const float new_y, const float old_x, const float old_y);
    template <bool track>
    void update_box_old_sectors(snake_bb &box);

    inline const body& get_head() const { return parts[0]; }
    inline float get_head_x() const { return parts[0].x; }
    inline float get_head_y() const { return parts[0].y; }
    inline float get_head_dx() const { return parts[0].x - parts[1].x; }
    inline float get_head_dy() const { return parts[0].y - parts[1].y; }

    std::shared_ptr<snake> get_ptr();
    snake_bb get_new_box() const;

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
    static const int move_step_distance = 42;
    static constexpr float tail_step_distance = 24.0f; // tail step eval for step dist = 42, k = 0.43
    static constexpr float rot_step_angle = 1.0f * move_step_distance / boost_speed * snake_angular_speed; // radians step per max acc resolution time
    static const long rot_step_interval = static_cast<long>(1000.0f * rot_step_angle / snake_angular_speed);
    static const long ai_step_interval = 1000;

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

    inline static float normalize_angle(float ang) {
        return ang - f_2pi * floorf( ang / f_2pi );
    }

private:

    long m_mov_ticks = 0;
    long m_rot_ticks = 0;
    long m_ai_ticks = 0;
};

#endif //SLITHER_GAME_SNAKE_HPP
