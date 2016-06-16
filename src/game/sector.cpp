#include "sector.hpp"

bool intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
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

bool intersect_circle(float p0_x, float p0_y, float p1_x, float p1_y, float r) {
    const float dx = p0_x - p1_x;
    const float dy = p0_y - p1_y;
    return dx * dx + dy * dy <= r * r;
}

bool snake_bb::find(sector *s) {
    for (sector *i : sectors) {
        if (i == s) {
            return true;
        }
    }

    return false;
}

size_t snake_bb::get_sectors_count() {
    return sectors.size();
}

size_t snake_bb::get_snakes_in_sectors_count() {
    size_t i = 0;
    for (const auto s : sectors) {
        i += s->m_snakes.size();
    }
    return i;
}

void snake_bb::reg_new_sector_if_missing(sector *s) {
    for (auto &ptr : new_sectors) {
        if (ptr == s) {
            return;
        }
    }

    new_sectors.push_back(s);
}

void snake_bb::reg_old_sector_if_missing(sector *s) {
    for (auto &ptr : old_sectors) {
        if (ptr == s) {
            return;
        }
    }

    old_sectors.push_back(s);
}

void sector::remove_snake(snake_id_t id) {
    const auto sn_end = m_snakes.end();
    for (auto sn_i = m_snakes.begin(); sn_i != sn_end; sn_i++) {
        if (sn_i->id == id) {
            m_snakes.erase(sn_i);
            break;
        }
    }
}

void sectors::init_sectors(const uint16_t sector_count_along_edge) {
    width = sector_count_along_edge;
    const size_t len = sector_count_along_edge * sector_count_along_edge;
    reserve(len);
    for (size_t i = 0; i < len; i ++) {
        push_back(sector{
                static_cast<uint8_t>(i % sector_count_along_edge),
                static_cast<uint8_t>(i / sector_count_along_edge),
                {}, {}
        });
    }
}

size_t sectors::get_index(uint16_t x, uint16_t y) {
    return y * width + x;
}

sector *sectors::get_sector(uint16_t x, uint16_t y) {
    return &operator[](get_index(x, y));
}



