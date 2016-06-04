#include "snake.hpp"
#include <iostream>

bool snake::tick(long dt) {
    uint8_t changes = 0;

    ticks += dt;

    const long speed_step_time = static_cast<long>(1000 * move_step_distance / speed);
    if (ticks >= speed_step_time) {
        const float timef = ticks / 1000.0f;
        ticks -= speed_step_time;

        if (angle != wangle) {
            float rotation = base_rotation_speed * timef;
            float dAngle = normalize_angle(wangle - angle);

            if (dAngle > M_PI) {
                dAngle -= 2.0f * M_PI;
            }

            if (fabs(dAngle) < rotation) {
                angle = wangle;
            } else {
                angle += rotation * (dAngle > 0 ? 1 : -1);
            }

            angle = normalize_angle(angle);
            changes |= change_angle;
        }

        // update parts
        const float move_dist = speed * timef;
        const size_t len = parts.size();

        // move head
        body& head = parts[0];
        body prev = head;

        head.x += cos(angle) * move_dist;
        head.y += sin(angle) * move_dist;
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
            const float move_coeff = parts_move_coeff * (++ j) / parts_start_move_count;
            pt.offset(move_coeff * (last.x - pt.x), move_coeff * (last.y - pt.y));

            prev = old;
        }

        // move tail
        for (size_t i = parts_skip_count + parts_start_move_count; i < len; ++ i) {
            body &pt = parts[i];
            const body last = parts[i - 1];
            const body old = pt;

            pt.from(prev);
            pt.offset(parts_move_coeff * (last.x - pt.x), parts_move_coeff * (last.y - pt.y));

            prev = old;
        }

        changes |= change_pos;

        // update speed
        const uint16_t wantedSpeed = acceleration ? boost_speed : base_move_speed;
        if (speed != wantedSpeed) {
            const float sgn = wantedSpeed > speed ? 1.0f : -1.0f;
            const uint16_t acc = static_cast<uint16_t>(speed_acceleration * timef);
            if (abs(wantedSpeed - speed) <= acc) {
                speed = wantedSpeed;
            } else {
                speed += sgn * acc;
            }
            changes |= change_speed;
        }

        ticks = 0;
    }

    if (changes > 0 && changes != update) {
        update |= changes;
        return true;
    }

    return false;
}

std::shared_ptr<snake> snake::getptr() {
    return shared_from_this();
}

void snake::flush() {
    update = 0;
}
