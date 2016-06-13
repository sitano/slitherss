#include "sector.hpp"

bool snake_bb::find(sector *s) {
    for (auto &i : sectors) {
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



