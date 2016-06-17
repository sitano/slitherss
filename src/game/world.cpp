#include "world.hpp"
#include <ctime>
#include <iostream>
#include <algorithm>

snake::ptr world::create_snake() {
    m_lastSnakeId ++;

    auto s = std::make_shared<snake>();
    s->id = m_lastSnakeId;
    s->name = "";
    s->skin = static_cast<uint8_t>(9 + next_random(21 - 9 + 1));
    s->speed = snake::base_move_speed;
    s->fullness = 0;

    float angle = world::f_2pi * next_randomf();
    float dist = 1000.0f + next_random(5000);
    uint16_t x = world_config::game_radius + dist * cosf(angle);
    uint16_t y = world_config::game_radius + dist * sinf(angle);
    angle = snake::normalize_angle(angle + f_pi);
    // const uint16_t half_radius = game_radius / 2;
    // uint16_t x = game_radius + next_random(game_radius) - half_radius;
    // uint16_t y = game_radius + next_random(game_radius) - half_radius;
    // todo: reserve snake.parts at least for sizeof(snake) bytes
    // todo: fix angles
    const int len = 1 /* head */ + 2 /* body min = 2 */ +
        std::max(m_config.snake_min_length, next_random(m_config.snake_average_length));

    for (int i = 0; i < len ; ++ i) {
        s->parts.push_back(body { 1.0f * x, 1.0f * y });
        x += cosf(angle) * snake::move_step_distance;
        y += sinf(angle) * snake::move_step_distance;
    }

    s->angle = snake::normalize_angle(angle + f_pi);
    s->wangle = snake::normalize_angle(angle + f_pi);

    s->box = s->get_new_box();
    s->update_box();
    s->update_box_sectors(m_sectors);

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
                s->update_box();
                s->update_box_sectors(m_sectors);
            }
        }
    }

    for (auto s: m_changes) {
        if (s->update & change_pos) {
            check_snake_bounds(s);
        }
    }
}

void world::check_snake_bounds(snake * const s) {
    static std::vector<snake_id_t> cs_cache;
    cs_cache.clear();

    // world bounds
    const body &head = s->get_head();
    if (head.distance_squared(world_config::game_radius, world_config::game_radius) >= world_config::death_radius * world_config::death_radius) {
        s->update |= change_dying;
        return;
    }

    // because we check after move being made
    auto check = s->parts[1];

    // check bound coverage
    const int16_t sx = static_cast<int16_t>(check.x / world_config::sector_size);
    const int16_t sy = static_cast<int16_t>(check.y / world_config::sector_size);
    static const int16_t width = 1;

    // 3x3 check head coverage
    const int16_t map_width_sectors = static_cast<int16_t>(world_config::sector_count_along_edge);
    for (int16_t j = sy - width; j <= sy + width; j ++) {
        for (int16_t i = sx - width; i <= sx + width; i++) {
            if (i >= 0 && i <= map_width_sectors && j >= 0 && j <= map_width_sectors) {
                sector *sec_ptr = m_sectors.get_sector(i, j);
                // check sector intersects head
                // todo radius from snake mass
                if (sec_ptr->intersect({ check.x, check.y, snake::move_step_distance * snake::move_step_distance })) {
                    // check sector snakes
                    for (const snake_bb &bb_ptr: sec_ptr->m_snakes) {
                        const snake *s2 = bb_ptr.snake_ptr;
                        if (s == s2) {
                            continue;
                        }

                        // check if snakes already checked
                        if (std::find(cs_cache.begin(), cs_cache.end(), s2->id) != cs_cache.end()) {
                            continue;
                        } else {
                            cs_cache.push_back(s2->id);
                        }

                        // check if snake intersects our bound box
                        if (!s->box.intersect(s2->box)) {
                            continue;
                        }

                        auto prev = s2->parts.begin();
                        auto bp_end = s2->parts.end();
                        for (auto bp_i = s2->parts.begin() + 1; bp_i != bp_end; bp_i++) {
                            // todo radius from snake mass
                            // weak body part check
                            if (intersect_circle(bp_i->x, bp_i->y, check.x, check.y, snake::move_step_distance * 2)) {
                                // check actual snake body
                                // todo radius from snake mass
                                const float r1 = 15.0f; // moving snake body radius
                                // todo radius from snake mass
                                const float r2 = 15.0f; // checked snake body radius
                                const float r = r1 + r2;
                                const float r_sqr = r * r;
                                if (distance_squared(bp_i->x, bp_i->y, prev->x, prev->y, check.x, check.y) <= r_sqr) {
                                    s->update |= change_dying;
                                    return;
                                }
                            }

                            prev++;
                        }
                    }
                }
            }
        }
    }

    // std::cout << "intersects " << i << ", sectors " << s->box.get_sectors_count() << ", snakes/in/s " << s->box.get_snakes_in_sectors_count() << std::endl;
}

void world::init(world_config config) {
    m_config = config;
    init_random();
    init_sectors();
    init_food();
    spawn_snakes(config.bots);
}

void world::init_sectors() {
    m_sectors.init_sectors(world_config::sector_count_along_edge);
}

void world::init_food() {
    for (sector &s: m_sectors) {
        const uint8_t cx = world_config::sector_count_along_edge / 2;
        const uint8_t cy = cx;
        const uint16_t dist = (s.x - cx) * (s.x - cx) + (s.y - cy) * (s.y - cy);
        const float dp = 1.0f - 1.0f * dist / (world_config::sector_count_along_edge * world_config::sector_count_along_edge);
        const size_t density = static_cast<size_t>(dp * 100);
        for (size_t i = 0; i < density; i ++) {
            s.m_food.push_back(food{
                    static_cast<uint16_t>(s.x * world_config::sector_size + next_random<uint16_t>(world_config::sector_size)),
                    static_cast<uint16_t>(s.y * world_config::sector_size + next_random<uint16_t>(world_config::sector_size)),
                    next_random<uint8_t>(255),
                    next_random<uint8_t>(255)
            });
        }
    }
}

void world::add_snake(snake::ptr ptr) {
    m_snakes.insert({ptr->id, ptr});
}

void world::remove_snake(snake_id_t id) {
    flush_changes(id);

    auto sn_i = get_snake(id);
    if (sn_i != m_snakes.end()) {
        for (auto sec_ptr : sn_i->second->box.sectors) {
            sec_ptr->remove_snake(id);
        }

        m_snakes.erase(id);
    }
}

world::snakes::iterator world::get_snake(snake_id_t id) {
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

void world::flush_changes(snake_id_t id) {
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
         << "\tgame_radius = " << world_config::game_radius
         << "\n\tmax_snake_parts = " << world_config::max_snake_parts
         << "\n\tsector_size = " << world_config::sector_size
         << "\n\tsector_count_along_edge = " << world_config::sector_count_along_edge
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

