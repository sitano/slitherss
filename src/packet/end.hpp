#ifndef SLITHER_PACKET_END_HPP
#define SLITHER_PACKET_END_HPP

#include "base.hpp"

struct packet_end : public packet_base {
    packet_end() : packet_base(packet_t_end) {}

    uint8_t status; // 3, int8, 0-2; 0 is normal death, 1 is new highscore of the day, 2 is unknown (disconnect??)

    size_t get_size() { return 4; }

    static const uint8_t status_death = 0;
    static const uint8_t status_highscore_otd = 1;
    static const uint8_t statuc_disconnect = 2;
};

std::ostream& operator<<(std::ostream & out, const packet_end & p) {
    out << static_cast<packet_base>(p);
    out << write_uint8(p.status);
    return out;
}

#endif //SLITHER_PACKET_END_HPP
