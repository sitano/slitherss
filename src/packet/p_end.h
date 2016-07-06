#ifndef SLITHER_PACKET_END_HPP
#define SLITHER_PACKET_END_HPP

#include "p_base.h"

// Sent when player died (dead/disconnect packet)
struct packet_end : public packet_base {
    packet_end() : packet_base(packet_t_end) {}
    explicit packet_end(uint8_t death_type) : packet_base(packet_t_end), status(death_type) {}

    uint8_t status = status_death; // 3, int8, 0-2; 0 is normal death, 1 is new highscore of the day, 2 is unknown (disconnect??)

    size_t get_size() const noexcept { return 4; }

    static const uint8_t status_death = 0;
    static const uint8_t status_highscore_otd = 1;
    static const uint8_t statuc_disconnect = 2;
};

// Sent when another snake dies by running into the player; not sent when the killer isn't the local player.
// Note: this packet is (currently) unused in the original client, so I can only guess what the variables mean.
struct packet_kill : public packet_base {
    packet_kill() : packet_base(packet_t_kill) {}

    uint16_t snakeId = 0; // 3-4	int16	killer snake id
    uint32_t kills = 0; // 5-7	int24	total number of kills

    size_t get_size() const noexcept { return 8; }
};

std::ostream& operator<<(std::ostream & out, const packet_end & p);
std::ostream& operator<<(std::ostream & out, const packet_kill & p);

#endif //SLITHER_PACKET_END_HPP
