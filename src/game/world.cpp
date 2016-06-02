#include "world.hpp"
#include <ctime>
#include <iostream>
#include <cmath>

std::shared_ptr<snake> world::create_snake() {
    m_lastSnakeId ++;

    uint32_t half_radius = game_radius / 2;

    auto s = new snake();
    s->id = m_lastSnakeId;
    s->name = "";
    s->color = static_cast<uint8_t>(9 + next_random(21 - 9 + 1));
    s->x = game_radius + next_random(game_radius) - half_radius;
    s->y = game_radius + next_random(game_radius) - half_radius;
    s->speed = snake::base_move_speed;

    s->fullness = 0.0f;

    const int len = 2 + next_random(10);
    for (int i = 0; i < len; ++ i) {
        s->parts.push_back(body { s->x, s->y });

        float angle = world::pi / 8.0f - world::pi / 4.0f * next_random() / RAND_MAX;
        s->angle = angle;
        s->wangle = angle;

        s->x += sinf(angle) * snake::move_step_distance / 2;
        s->y += cosf(angle) * snake::move_step_distance / 2;
    }

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
