#include "snake.hpp"

bool snake::tick(long dt) {
    uint8_t changes = 0;

    if (update & (change_dying | change_dead)) {
        return false;
    }

    if (bot) {
        m_ai_ticks += dt;
        if (m_ai_ticks > ai_step_interval) {
            const long frames = m_ai_ticks / ai_step_interval;
            const long frames_ticks = frames * ai_step_interval;

            tick_ai(frames);

            m_ai_ticks -= frames_ticks;
        }
    }

    // rotation
    if (angle != wangle) {
        m_rot_ticks += dt;
        if (m_rot_ticks >= rot_step_interval) {
            const long frames = m_rot_ticks / rot_step_interval;
            const long frames_ticks = frames * rot_step_interval;
            const float rotation = snake_angular_speed * frames_ticks / 1000.0f;
            float dAngle = normalize_angle(wangle - angle);

            if (dAngle > f_pi) {
                dAngle -= f_2pi;
            }

            if (fabsf(dAngle) < rotation) {
                angle = wangle;
            } else {
                angle += rotation * (dAngle > 0 ? 1.0f : -1.0f);
            }

            angle = normalize_angle(angle);

            changes |= change_angle;
            m_rot_ticks -= frames_ticks;
        }
    }

    // movement
    m_mov_ticks += dt;
    const long mov_frame_interval = 1000 * move_step_distance / speed;
    if (m_mov_ticks >= mov_frame_interval) {
        const long frames = m_mov_ticks / mov_frame_interval;
        const long frames_ticks = frames * mov_frame_interval;
        const float move_dist = speed * frames_ticks / 1000.0f;
        const size_t len = parts.size();

        // move head
        body& head = parts[0];
        body prev = head;
        head.x += cosf(angle) * move_dist;
        head.y += sinf(angle) * move_dist;

        for (size_t i = 1; i < len && i < parts_skip_count; ++ i) {
            const body old = parts[i];
            parts[i] = prev;
            prev = old;
        }

        // move intermediate
        for (size_t i = parts_skip_count, j = 0; i < len && i < parts_skip_count + parts_start_move_count; ++ i) {
            body &pt = parts[i];
            const body last = parts[i - 1];
            const body old = pt;

            pt.from(prev);
            const float move_coeff = snake_tail_k * (++ j) / parts_start_move_count;
            pt.offset(move_coeff * (last.x - pt.x), move_coeff * (last.y - pt.y));

            prev = old;
        }

        // move tail
        for (size_t i = parts_skip_count + parts_start_move_count; i < len; ++ i) {
            body &pt = parts[i];
            const body last = parts[i - 1];
            const body old = pt;

            pt.from(prev);
            pt.offset(snake_tail_k * (last.x - pt.x), snake_tail_k * (last.y - pt.y));

            prev = old;
        }

        changes |= change_pos;

        // update speed
        const uint16_t wantedSpeed = acceleration ? boost_speed : base_move_speed;
        if (speed != wantedSpeed) {
            const float sgn = wantedSpeed > speed ? 1.0f : -1.0f;
            const uint16_t acc = static_cast<uint16_t>(speed_acceleration * frames_ticks / 1000.0f);
            if (abs(wantedSpeed - speed) <= acc) {
                speed = wantedSpeed;
            } else {
                speed += sgn * acc;
            }
            changes |= change_speed;
        }

        m_mov_ticks -= frames_ticks;
    }

    if (changes > 0 && changes != update) {
        update |= changes;
        return true;
    }

    return false;
}

std::shared_ptr<snake> snake::get_ptr() {
    return shared_from_this();
}

void snake::update_box() {
    float x = 0.0f;
    float y = 0.0f;

    // calculate center mass
    for (const body &p : parts) {
        x += p.x;
        y += p.y;
    }

    x /= parts.size();
    y /= parts.size();

    box.x = x;
    box.y = y;

    // calculate radius^2
    float r2 = 0.0f;
    for (const body &p : parts) {
        const float r_tmp = p.distance_squared(x, y);
        if (r_tmp > r2) {
            r2 = r_tmp;
        }
    }

    box.r2 = 100 * 100 /* minimal bb */ + r2;
}

void snake::update_box_sectors(sectors &ss) {
    // remove snake from sectors which passed by
    auto sec_end = box.sectors.end();
    for (auto sec_i = box.sectors.begin(); sec_i != sec_end; sec_i ++) {
        sector *sec = *sec_i;
        if (!sec->intersect(box)) {
            // clean up snake
            sec->remove_snake(id);
            // pop sector
            box.sectors.erase(sec_i);
            box.reg_old_sector_if_missing(sec);
            sec_i --;
            sec_end = box.sectors.end();
        }
    }

    // register snake to new sectors
    const int16_t width_2 = static_cast<int16_t>(1 + linear_sqrt(box.r2 / world_config::sector_size / world_config::sector_size));
    const int16_t sx = static_cast<int16_t>(box.x / world_config::sector_size);
    const int16_t sy = static_cast<int16_t>(box.y / world_config::sector_size);

    const int16_t map_width_sectors = static_cast<int16_t>(world_config::sector_count_along_edge);
    for (int16_t j = sy - width_2; j <= sy + width_2; j ++) {
        for (int16_t i = sx - width_2; i <= sx + width_2; i ++) {
            if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
                sector *new_sector = ss.get_sector(i, j);
                if (!box.any_of(new_sector) && new_sector->intersect(box)) {
                    new_sector->m_snakes.push_back(box);
                    box.sectors.push_back(new_sector);
                    box.reg_new_sector_if_missing(new_sector);
                }
            }
        }
    }
}

snake_bb snake::get_new_box() const {
    return { { get_head_x(), get_head_y(), 0 }, id, this, {} };
}

void snake::tick_ai(long frames) {
    for (auto i = 0; i < frames; i ++) {
        // 1. calc angle of radius vector
        float ai_angle = atan2f(get_head_y() - world_config::game_radius, get_head_x() - world_config::game_radius);
        // 2. add pi_2
        ai_angle = normalize_angle(ai_angle + f_pi / 2.0f);
        // 3. set
        if (fabsf(wangle - ai_angle) > 0.01f) {
            wangle = ai_angle;
            update |= change_wangle;
        }
    }
}

float linear_sqrt(float x) {
    static constexpr float d = 1.0f / 6.0f;
    return 3.0f + (x - 9.0f) * d;
}
