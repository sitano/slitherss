#include "world.hpp"
#include <ctime>
#include <iostream>

std::shared_ptr<snake> world::create_snake() {
    m_lastSnakeId ++;

    float angle = pi_mul_2 * next_random(255) / 256.0f;

    auto s = new snake();
    s->id = m_lastSnakeId;
    s->name = "";
    s->color = static_cast<uint8_t>(9 + next_random(21 - 9 + 1));
    s->x = game_radius + next_random(1000) - 500;
    s->y = game_radius + next_random(1000) - 500;
    s->speed = 5.79f;
    s->angle = angle;
    s->wangle = angle;
    s->fullness = 0.0f;
    s->parts = {
            {s->x, s->y},
            {s->x, s->y}
    };

    return std::shared_ptr<snake>(s);
}

void world::init_random() {
    std::srand(std::time(nullptr));
}

int world::next_random() {
    return std::rand();
}

template<typename T>
T world::next_random(T base) {
    return static_cast<T>(next_random() % base);
}

void world::tick(long dt) {
    m_ticks += dt;
    long virtual_frames = m_ticks / virtual_frame_time_ms;
    if (virtual_frames > 0) {
        m_ticks -= virtual_frames * virtual_frame_time_ms;
        m_virtual_frames += virtual_frames;
    }
}

void world::init() {
    init_random();
}
