#include "snake.hpp"

uint8_t snake::tick(long dt) {
    uint8_t changes = 0;

    ticks += dt;

    const long speed_step_time = static_cast<long>(speed / move_step_distance);
    if (ticks >= speed_step_time) {
        ticks -= speed_step_time;

        float timef = ticks / 1000.0f;
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
        body prev = parts.front();
        body old = prev;

        // move head
        const float move_dist = speed * timef;

        prev.x += cos(angle) * move_dist;
        prev.y += sin(angle) * move_dist;

        // todo snake x, y ???
        // todo copy of body parts???
        // distance is fixed
        body tmp;
        float move_coeff = 0.0f;

        for (size_t i = 1, j = 0; i < parts.size(); ++ i) {
            body &pt = parts[i];
            tmp = pt;
            pt = old;

            if (i >= parts_skip_count) {
                if (j < parts_start_move_count) {
                    move_coeff = parts_move_coeff * (++ j) / parts_start_move_count;
                }
                pt.offset(move_coeff * (prev.x - pt.x), move_coeff * (prev.y - pt.y));
            }
            prev = pt;
            old = tmp; //restore original position;
        }
        changes |= change_pos;

        // update speed
        float wantedSpeed = acceleration ? boost_speed : base_move_speed;
        if (speed != wantedSpeed) {
            float sgn = wantedSpeed > speed ? 1.0f : -1.0f;
            float acc = speed_acceleration * timef;
            if (fabs(wantedSpeed - speed) <= acc) {
                speed = wantedSpeed;
            } else {
                speed += sgn * acc;
            }
            changes |= change_speed;
        }

        ticks = 0;
    }

    return changes;
}


