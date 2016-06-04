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
    change_speed = 4
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
};

struct snake : std::enable_shared_from_this<snake> {
    typedef uint16_t snake_id_t;
    typedef std::shared_ptr<snake> ptr;

    snake_id_t id;

    uint8_t color;
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
    void flush();

    inline float get_head_x() const { return parts[0].x; }
    inline float get_head_y() const { return parts[0].y; }
    inline float get_head_dx() const { return parts[0].x - parts[1].x; }
    inline float get_head_dy() const { return parts[0].y - parts[1].y; }

    std::shared_ptr<snake> getptr();

    static constexpr float spangdv = 4.8f;
    static constexpr float nsp1 = 5.39f;
    static constexpr float nsp2 = 0.4f;
    static constexpr float nsp3 = 14.0f;

    static constexpr float snake_ang_speed = 0.033f;
    static constexpr float prey_ang_speed = 0.028f;
    static constexpr float snake_tail_k = 0.43f;

    static const uint16_t base_move_speed = 185; // pixel in second (convert:  1000*sp/32)
    static const uint16_t boost_speed = 448; // pixel in second (convert:  1000*sp/32)
    static const uint16_t speed_acceleration = 1000; // pixel in second
    static constexpr float base_rotation_speed = 4.125f; // radian in second (convert:  1000*MAMU/8)
    static constexpr float parts_move_coeff = 0.43f;

    static const int parts_skip_count = 3;
    static const int parts_start_move_count = 4;
    static const int move_step_distance = 42;

    static constexpr float f_pi = 3.14159265358979323846f;
    static constexpr float f_2pi = 2.0f * f_pi;

private:

    long ticks = 0;
};

inline float normalize_angle(float angle) {
    return angle - snake::f_2pi * floorf( angle / snake::f_2pi );
}

#endif //SLITHER_GAME_SNAKE_HPP
