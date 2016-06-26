#include "snake.hpp"

#include <iostream>

bool snake::tick(long dt, sectors &ss) {
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
    const long mov_frame_interval = 1000 * world_config::move_step_distance / speed;
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

        sbb.update_box_new_sectors(ss, world_config::sector_size / 2, head.x, head.y, prev.x, prev.y);
        if (!bot) {
            vp.update_box_new_sectors(ss, head.x, head.y, prev.x, prev.y);
        }

        // bound box
        float bbx = head.x;
        float bby = head.y;

        for (size_t i = 1; i < len && i < parts_skip_count; ++ i) {
            const body old = parts[i];
            parts[i] = prev;
            bbx += prev.x;
            bby += prev.y;
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

            bbx += pt.x;
            bby += pt.y;
            prev = old;
        }

        // move tail
        for (size_t i = parts_skip_count + parts_start_move_count, j = 0; i < len; ++ i) {
            body &pt = parts[i];
            const body last = parts[i - 1];
            const body old = pt;

            pt.from(prev);
            pt.offset(snake_tail_k * (last.x - pt.x), snake_tail_k * (last.y - pt.y));

            // as far as having step dist = 42, k = 0.43, sec. size = 300, this could be 300 / 24.0f, with radius 150
            static const size_t tail_step = static_cast<size_t>(world_config::sector_size / tail_step_distance);
            if (j + tail_step >= i) {
                sbb.update_box_new_sectors(ss, world_config::sector_size / 2, pt.x, pt.y, old.x, old.y);
                j = i;
            }

            bbx += pt.x;
            bby += pt.y;
            prev = old;
        }

        changes |= change_pos;

        // update bb
        sbb.x = bbx / parts.size();
        sbb.y = bby / parts.size();
        vp.x = head.x;
        vp.y = head.y;
        update_box_radius();
        sbb.update_box_old_sectors();
        if (!bot) {
            vp.update_box_old_sectors();
        }
        update_eaten_food(ss);

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

void snake::update_box_center() {
    float x = 0.0f;
    float y = 0.0f;

    // calculate center mass
    for (const body &p : parts) {
        x += p.x;
        y += p.y;
    }

    x /= parts.size();
    y /= parts.size();

    sbb.x = x;
    sbb.y = y;

    vp.x = get_head_x();
    vp.y = get_head_y();
}

void snake::update_box_radius() {
    // calculate bb radius, len eval for step dist = 42, k = 0.43
    // parts ..  1  ..  2  ..  3   ..  4   ..  5   ..  6   ..  7  .. tail by 24.0f
    float d = 42.0f + 42.0f + 42.0f + 37.7f + 37.7f + 33.0f + 28.5f;

    if (parts.size() > 8) {
        d += tail_step_distance * (parts.size() - 8);
    }

    // reserve 1 step ahead of the snake radius
    sbb.r = (d + world_config::move_step_distance) / 2.0f;

    vp.r = world_config::sector_diag_size * 3.0f;
}

void snake::init_box_new_sectors(sectors &ss) {
    body& head = parts[0];
    sbb.update_box_new_sectors(ss, world_config::sector_size / 2, head.x, head.y, 0.0f, 0.0f);

    if (!bot) {
        vp.update_box_new_sectors(ss, head.x, head.y, 0.0f, 0.0f);
    }

    const size_t len = parts.size();
    // as far as having step dist = 42, k = 0.43, sec. size = 300, this could be 300 / 24.0f, with radius 150
    static const size_t tail_step = static_cast<size_t>(world_config::sector_size / tail_step_distance);
    for (size_t i = 3; i < len; i += tail_step) {
        body &pt = parts[i];
        sbb.update_box_new_sectors(ss, world_config::sector_size / 2, pt.x, pt.y, 0.0f, 0.0f);
    }
}

void snake::update_eaten_food(sectors &ss) {
    const uint16_t hx = static_cast<uint16_t>(get_head_x());
    const uint16_t hy = static_cast<uint16_t>(get_head_y());
    const uint16_t r = static_cast<uint16_t>(14 + get_snake_body_part_radius() + world_config::move_step_distance);
    const uint32_t r2 = r * r;

    const uint16_t sx = hx / world_config::sector_size;
    const uint16_t sy = hy / world_config::sector_size;

    // head sector
    {
        sector *sc = ss.get_sector(sx, sy);
        auto begin = sc->m_food.begin();
        auto i = sc->find_closest_food(hx);
        // to left
        {
            auto left = i - 1;
            while (left >= begin && distance_squared(left->x, left->y, hx, hy) <= r2) {
                // std::cout << "eaten food <left> x = " << left->x << ", y = " << left->y << std::endl;
                fullness += i->size;
                eaten.push_back(*left);
                sc->remove_food(left);
                i--;
                left--;
            }
        }
        // to right
        {
            auto end = sc->m_food.end();
            while (i < end && distance_squared(i->x, i->y, hx, hy) <= r2) {
                // std::cout << "eaten food <right> x = " << i->x << ", y = " << i->y << std::endl;
                fullness += i->size;
                eaten.push_back(*i);
                sc->remove_food(i);
                end--;
            }
        }
    }
}

bb snake::get_new_box() const {
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

bool snake::intersect(bb_pos foe,
                      std::vector<body>::const_iterator prev,
                      std::vector<body>::const_iterator i,
                      std::vector<body>::const_iterator end) const {
    while (i != end) {
        // weak body part check
        if (intersect_circle(i->x, i->y, foe.x, foe.y, world_config::move_step_distance * 2)) {
            const float r = foe.r + get_snake_body_part_radius();

            // check actual snake body part
            if (intersect_circle(i->x, i->y, foe.x, foe.y, r) ||
                    intersect_circle(prev->x, prev->y, foe.x, foe.y, r) ||
                    intersect_circle(i->x + (prev->x - i->x) / 2.0f, i->y + (prev->y - i->y) / 2.0f, foe.x, foe.y, r)) {
                return true;
            }
        }

        ++prev;
        ++i;
    }

    return false;
}

bool snake::intersect(bb_pos foe) const {
    static const size_t head_size = 8;
    static const size_t tail_step = static_cast<size_t>(world_config::sector_size / tail_step_distance);
    static const size_t tail_step_half = tail_step / 2;
    const size_t len = parts.size();

    if (len <= head_size + tail_step) {
        return intersect(foe, parts.begin(), parts.begin() + 1, parts.end());
    } else {
        // calculate bb radius, len eval for step dist = 42, k = 0.43
        // parts ..  1  ..  2  ..  3  ..  4  ..  5  ..  6  ..  7  .. tail by 24.0f
        // head center will be i = 3, len [0 .. 3] = 42 * 3 = 126, len [3 .. 7] = 136.9, both < sector_size / 2 = 150
        auto head = parts[3];
        if (intersect_circle(head.x, head.y, foe.x, foe.y, world_config::sector_size / 2)) {
            if (intersect(foe, parts.begin(), parts.begin() + 1, parts.begin() + 9)) {
                return true;
            }
        }

        // first tail sector center will be... skip 8 + tail_step / 2
        auto end = parts.end();
        for (auto i = parts.begin() + 7 + tail_step_half; i < end; i += tail_step) {
            if (intersect_circle(i->x, i->y, foe.x, foe.y, world_config::sector_size / 2)) {
                auto start = i - tail_step_half;
                auto last = i + tail_step_half;
                if (last > end) {
                    last = end;
                }
                if (intersect(foe, i - 1, start, last)) {
                    return true;
                }
            }
        }
    }

    return false;
}

void snake::increase_snake() {
    parts.push_back(parts.back());
}

float snake::get_snake_body_part_radius() const {
    // todo radius from snake mass
    return 14.0f;
}




