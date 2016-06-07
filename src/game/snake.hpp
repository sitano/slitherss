#ifndef SLITHER_GAME_SNAKE_HPP
#define SLITHER_GAME_SNAKE_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

enum snake_changes : uint8_t {
    change_pos = 1,
    change_angle = 2,
    change_wangle = 4,
    change_speed = 8,
};

struct body {
    float x;
    float y;

    // todo: maybe we could have it int and relative coords
    inline void from(const body& p) {
        x = p.x;
        y = p.y;
    }

    inline void offset(float dx, float dy) {
        x += dx;
        y += dy;
    }
};

struct snake : std::enable_shared_from_this<snake> {
    typedef uint16_t snake_id_t;
    typedef std::shared_ptr<snake> ptr;

    snake_id_t id;

    uint8_t skin;
    uint8_t update;
    bool acceleration;

    std::string name;

    // pixels / seconds, base ~185 [px/s]
    uint16_t speed;

    float angle;
    float wangle;

    // 0 - 100, 0 - hungry, 100 - full
    uint8_t fullness;

    std::vector<body> parts;

    bool tick(long dt);

    inline float get_head_x() const { return parts[0].x; }
    inline float get_head_y() const { return parts[0].y; }
    inline float get_head_dx() const { return parts[0].x - parts[1].x; }
    inline float get_head_dy() const { return parts[0].y - parts[1].y; }

    std::shared_ptr<snake> getptr();

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
    static constexpr float rot_step_angle = 1.0f * move_step_distance / boost_speed * snake_angular_speed; // radians step per max acc resolution time
    static const long rot_step_interval = static_cast<long>(1000.0f * rot_step_angle / snake_angular_speed);

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

    inline static float normalize_angle(float ang) {
        return ang - f_2pi * floorf( ang / f_2pi );
    }

private:

    long m_mov_ticks = 0;
    long m_rot_ticks = 0;
};



#endif //SLITHER_GAME_SNAKE_HPP
