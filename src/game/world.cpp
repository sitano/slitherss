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

    float angle = world::f_2pi * next_randomf();
    float dist = 1000.0f + next_random(5000);
    uint16_t x = game_radius + dist * cosf(angle);
    uint16_t y = game_radius + dist * sinf(angle);
    angle = snake::normalize_angle(angle + f_pi);
    // const uint16_t half_radius = game_radius / 2;
    // uint16_t x = game_radius + next_random(game_radius) - half_radius;
    // uint16_t y = game_radius + next_random(game_radius) - half_radius;
    // todo: reserve snake.parts at least for sizeof(snake) bytes
    // todo: fix angles
    const int len = 1 /* head */ + 2 /* body min = 2 */ + next_random(10);
    for (int i = 0; i < len; ++ i) {
        s->parts.push_back(body { 1.0f * x, 1.0f * y });

        x += cosf(angle) * snake::move_step_distance;
        y += sinf(angle) * snake::move_step_distance;
    }

    s->angle = snake::normalize_angle(angle + f_pi);
    s->wangle = snake::normalize_angle(angle + f_pi);
    s->box = s->get_new_box();

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
        snake * const s = pair.second.get();

        if (s->tick(dt)) {
            m_changes.push_back(s);

            if (s->update & change_pos) {
                check_snake_bounds(s);
            }
        }
    }
}

void world::check_snake_bounds(snake * const s) {
    // world bounds
    const body &head = s->get_head();
    if (head.distance_squared(game_radius, game_radius) >= death_radius * death_radius) {
        s->update |= change_dying;
    }

    // todo fix naive snakes bounds
    auto h1 = s->parts[0];
    auto h2 = s->parts[2];
    for (auto ptr: m_snakes) {
        snake *s2 = ptr.second.get();
        if (s == s2) {
            continue;
        }

        if (!s->intersect_snake_box(s2)) {
            continue;
        }

        body prev = s2->parts.front();
        auto end = s2->parts.end();
        for (auto i = s2->parts.begin() + 1; i != end; i ++) {
            if (intersect(h1.x, h1.y, h2.x, h2.y, prev.x, prev.y, i->x, i->y)) {
                // hit
                s->update |= change_dying;
                return;
            }
            prev = *i;
        }
    }
}

void world::init() {
    init_random();
    init_sectors();
    init_food();
}

void world::init_sectors() {
    const size_t len = sector_count_along_edge * sector_count_along_edge;
    m_sectors.reserve(len);
    for (size_t i = 0; i < len; i ++) {
        m_sectors.push_back(sector{
                static_cast<uint8_t>(i % sector_count_along_edge),
                static_cast<uint8_t>(i / sector_count_along_edge),
                {}, {}
        });
    }
}

void world::init_food() {
    for (auto s: m_sectors) {
        const uint8_t cx = sector_count_along_edge / 2;
        const uint8_t cy = cx;
        const uint16_t dist = (s.x - cx) * (s.x - cx) + (s.y - cy) * (s.y - cy);
        const float dp = 1.0f - 1.0f * dist / (sector_count_along_edge * sector_count_along_edge);
        const size_t density = static_cast<size_t>(dp * 100);
        for (size_t i = 0; i < density; i ++) {
            s.m_food.push_back(food{
                    static_cast<uint16_t>(s.x * sector_size + next_random<uint16_t>(sector_size)),
                    static_cast<uint16_t>(s.y * sector_size + next_random<uint16_t>(sector_size)),
                    next_random<uint8_t>(255),
                    next_random<uint8_t>(255)
            });
        }
    }
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

world::v_ids &world::get_dead() {
    return m_dead;
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

bool intersect(float p0_x, float p0_y, float p1_x, float p1_y,
               float p2_x, float p2_y, float p3_x, float p3_y) {
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    const float d = (-s2_x * s1_y + s1_x * s2_y);
    static const float epsilon = 0.0001f;
    if (d <= epsilon && d >= -epsilon) {
        return false;
    }

    // todo check is it better to have 2 more mul, then 1 branch
    const float s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
    if (s < 0 || s > d) {
        return false;
    }

    const float t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
    if (t < 0 || t > d) {
        return false;
    }

    return true;
}
