#include "world.hpp"
#include <ctime>
#include <iostream>
#include <cmath>

snake::ptr world::create_snake() {
    m_lastSnakeId ++;

    const auto s = std::make_shared<snake>();
    s->id = m_lastSnakeId;
    s->name = "";
    s->color = static_cast<uint8_t>(9 + next_random(21 - 9 + 1));
    s->speed = snake::base_move_speed;
    s->fullness = 0;

    const uint16_t half_radius = game_radius / 2;
    uint16_t x = game_radius + next_random(game_radius) - half_radius;
    uint16_t y = game_radius + next_random(game_radius) - half_radius;
    // todo: reserve snake.parts at least for sizeof(snake) bytes
    // todo: fix angles
    float angle = world::f_2pi * next_randomf();
    const int len = 2 + next_random(10);
    for (int i = 0; i < len; ++ i) {
        s->parts.push_back(body { 1.0f * x, 1.0f * y });

        x += cosf(angle) * snake::move_step_distance / 2;
        y += sinf(angle) * snake::move_step_distance / 2;

        std::cout << "x = " << x << " y = " << y << std::endl;
    }

    s->angle = normalize_angle(angle + f_pi);
    s->wangle = normalize_angle(angle + f_pi);
    std::cout << "angle = " << angle << std::endl;
    std::cout << "wangle = " << s->wangle << std::endl;
    return s;
}

void world::init_random() {
    std::srand(std::time(nullptr));
}

int world::next_random() {
    return std::rand();
}

float world::next_randomf() {
    return 1.0f * std::rand() / RAND_MAX;
}

template<typename T>
T world::next_random(T base) {
    return static_cast<T>(next_random() % base);
}

void world::tick(long dt) {
    m_ticks += dt;
    const long virtual_frames = m_ticks / virtual_frame_time_ms;
    if (virtual_frames > 0) {
        const long virtual_frames_time = virtual_frames * virtual_frame_time_ms;
        tick_snakes(virtual_frames_time);

        m_ticks -= virtual_frames_time;
        m_virtual_frames += virtual_frames;
    }
}

void world::tick_snakes(long dt) {
    for (auto pair: m_snakes) {
        if (pair.second->tick(dt)) {
            m_changes.push_back(pair.second.get());
        }
    }
}

void world::init() {
    init_random();
    // todo: init sectors
}

void world::add_snake(snake::ptr ptr) {
    m_snakes.insert({ptr->id, ptr});
}

void world::remove_snake(snake::snake_id_t id) {
    flush_changes(id);
    m_snakes.erase(id);
}

world::snakes::iterator world::get_snake(snake::snake_id_t id) {
    return m_snakes.find(id);
}

std::vector<snake *>& world::get_changes() {
    return m_changes;
}

void world::flush_changes() {
    m_changes.clear();
}

void world::flush_changes(snake::snake_id_t id) {
    for (auto ptr = m_changes.begin(); ptr != m_changes.end(); ++ ptr) {
        if ((*ptr)->id == id) {
            m_changes.erase(ptr);
        }
    }
}






