#include "world.hpp"
#include <ctime>
#include <iostream>
#include <cmath>

snake::ptr world::create_snake() {
    m_lastSnakeId ++;

    const auto s = std::make_shared<snake>();
    s->id = m_lastSnakeId;
    s->name = "";
    s->skin = static_cast<uint8_t>(9 + next_random(21 - 9 + 1));
    s->speed = snake::base_move_speed;
    s->fullness = 0;

    // uint16_t x = game_radius + next_random(1000) - 500;
    // uint16_t y = game_radius + next_random(1000) - 500;
    const uint16_t half_radius = game_radius / 2;
    uint16_t x = game_radius + next_random(game_radius) - half_radius;
    uint16_t y = game_radius + next_random(game_radius) - half_radius;
    // todo: reserve snake.parts at least for sizeof(snake) bytes
    // todo: fix angles
    float angle = world::f_2pi * next_randomf();
    const int len = 1 /* head */ + 2 /* body min = 2 */ + next_random(10);
    for (int i = 0; i < len; ++ i) {
        s->parts.push_back(body { 1.0f * x, 1.0f * y });

        x += cosf(angle) * snake::move_step_distance;
        y += sinf(angle) * snake::move_step_distance;
    }

    s->angle = snake::normalize_angle(angle + f_pi);
    s->wangle = snake::normalize_angle(angle + f_pi);

    return s;
}

snake::ptr world::create_snake_bot() {
    snake::ptr ptr = create_snake();
    ptr->bot = true;
    return ptr;
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
            snake * const s = pair.second.get();

            m_changes.push_back(s);

            if (s->update & change_pos) {
                check_snake_bounds(s);
            }
        }
    }
}

void world::check_snake_bounds(snake * const s) {
    const body &head = s->get_head();
    if (head.distance_squared(game_radius, game_radius) >= death_radius * death_radius) {
        s->update |= change_dying;
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

world::snakes &world::get_snakes() {
    return m_snakes;
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

void world::spawn_snakes(const int snakes) {
    for (int i = 0; i < snakes; i++) {
        add_snake(create_snake_bot());
    }
}

std::ostream &operator<<(std::ostream &out, const world &w) {
    return out
         << "\tgame_radius = " << w.game_radius
         << "\n\tmax_snake_parts = " << w.max_snake_parts
         << "\n\tsector_size = " << w.sector_size
         << "\n\tsector_count_along_edge = " << w.sector_count_along_edge
         << "\n\tvirtual_frame_time_ms = " << w.virtual_frame_time_ms
         << "\n\tprotocol_version = " << static_cast<long>(w.protocol_version)
         << "\n\tspangdv = " << snake::spangdv
         << "\n\tnsp1 = " << snake::nsp1
         << "\n\tnsp2 = " << snake::nsp2
         << "\n\tnsp3 = " << snake::nsp3
         << "\n\tbase_move_speed = " << snake::base_move_speed
         << "\n\tboost_speed = " << snake::boost_speed
         << "\n\tspeed_acceleration = " << snake::speed_acceleration
         << "\n\tprey_angular_speed = " << snake::prey_angular_speed
         << "\n\tsnake_angular_speed = " << snake::snake_angular_speed
         << "\n\tsnake_tail_k = " << snake::snake_tail_k
         << "\n\tparts_skip_count = " << snake::parts_skip_count
         << "\n\tparts_start_move_count = " << snake::parts_start_move_count
         << "\n\tmove_step_distance = " << snake::move_step_distance
         << "\n\trot_step_angle = " << snake::rot_step_angle;
}
