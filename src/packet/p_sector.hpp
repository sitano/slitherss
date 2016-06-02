#ifndef SLITHER_PACKET_SECTOR_HPP
#define SLITHER_PACKET_SECTOR_HPP

#include "p_base.hpp"

struct packet_sector : public packet_base {
    explicit packet_sector(packet_t t) : packet_base(t) {}

    uint8_t x = 0;  // 3, int8, x-coordinate of the sector
    uint8_t y = 0; // 4, int8, y-coordinate of the sector

    size_t get_size() { return 5; }
};

struct packet_add_sector : public packet_sector {
    packet_add_sector() : packet_sector(packet_t_add_sector) { }
};

struct packet_remove_sector : public packet_sector {
    packet_remove_sector() : packet_sector(packet_t_rem_sector) { }
};

std::ostream& operator<<(std::ostream & out, const packet_sector & p);

#endif //SLITHER_PACKET_SECTOR_HPP
