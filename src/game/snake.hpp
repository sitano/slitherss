#ifndef SLITHER_GAME_SNAKE_HPP
#define SLITHER_GAME_SNAKE_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>

enum snake_changes : uint8_t {
    change_pos = 1,
    change_angle = 2,
    change_speed = 4
};

struct body {
    float x;
    float y;

    inline void offset(float dx, float dy) {
        x += dx;
        y += dy;
    }
};

struct snake {
    uint16_t id;

    bool acceleration;
    uint8_t color;

    std::string name;

    float x;
    float y;

    float speed;

    float angle;
    float wangle;

    float fullness;

    std::vector<body> parts;

    uint8_t tick(long dt);

    static constexpr float spangdv = 4.8f;
    static constexpr float nsp1 = 5.39f;
    static constexpr float nsp2 = 0.4f;
    static constexpr float nsp3 = 14.0f;

    static constexpr float snake_ang_speed = 0.033f;
    static constexpr float prey_ang_speed = 0.028f;
    static constexpr float snake_tail_k = 0.43f;

    static const int base_move_speed = 185; // pixel in second (convert:  1000*sp/32)
    static const int boost_speed = 448; // pixel in second (convert:  1000*sp/32)
    static const int speed_acceleration = 1000; // pixel in second
    static constexpr float base_rotation_speed = 4.125f; // radian in second (convert:  1000*MAMU/8)
    static constexpr float parts_move_coeff = 0.43f;

    static const int parts_skip_count = 3;
    static const int parts_start_move_count = 4;
    static const int move_step_distance = 42;

private:

    long ticks = 0;
};

inline float normalize_angle(float angle) {
    return angle - (2.0f * M_PI) * floor( angle / (2.0f * M_PI) );
}

#endif //SLITHER_GAME_SNAKE_HPP
