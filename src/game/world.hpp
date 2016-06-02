#ifndef SLITHER_GAME_WORLD_HPP
#define SLITHER_GAME_WORLD_HPP

#include "snake.hpp"
#include <memory>
#include <cstdint>

class world {
public:

    void init();
    void tick(long dt);

    std::shared_ptr<snake> create_snake();

    void init_random();
    int next_random();
    template <typename T> T next_random(T base);

    static const uint32_t game_radius = 21600;
    static const uint16_t max_snake_parts = 411;
    static const uint16_t sector_size = 300;
    static const uint16_t sector_count_along_edge = 144;

    static const long virtual_frame_time_ms = 8;

    static constexpr float spangdv = 4.8f;
    static constexpr float nsp1 = 5.39f;
    static constexpr float nsp2 = 0.4f;
    static constexpr float nsp3 = 14.0f;

    static constexpr float snake_ang_speed = 0.033f;
    static constexpr float prey_ang_speed = 0.028f;
    static constexpr float snake_tail_k = 0.43f;

    static const uint8_t protocol_version = 8;

    static constexpr float pi = 3.14159265358979323846f;
    static constexpr float pi_mul_2 = 2.0f * 3.14159265358979323846f;

private:
    // todo snakes
    // todo sectors
    // todo fixed point arithmetic
    // todo pools
    // todo sorted checker

    uint16_t m_lastSnakeId = 0;
    long m_ticks = 0;
    uint32_t m_virtual_frames = 0;
};

#endif //SLITHER_GAME_WORLD_HPP
