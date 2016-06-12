#include "sector.hpp"

size_t snake_bb::get_snakes_in_sectors_count() {
    size_t i = 0;
    for (const auto s : sectors) {
        i += s->m_snakes.size();
    }
    return i;
}
