#ifndef SLITHER_PACKET_INIT_HPP
#define SLITHER_PACKET_INIT_HPP

#include "p_base.hpp"

struct packet_init : public packet_base {
    packet_init() : packet_base(packet_t_init) {}

    uint32_t game_radius = 21600; // 3-5, int24, Game Radius = 21600
    uint16_t max_snake_parts = 411; // 6-7, int16, mscps (maximum snake length in body parts units) = 411
    uint16_t sector_size = 300; // 8-9, int16, sector_size = 480
    uint16_t sector_count_along_edge = 144; // 10-11, int16, sector_count_along_edge (unused in the game-code) = 130
    float spangdv = 4.8f; // 12, int8, spangdv (value / 10) (coef. to calculate angular speed change depending snake speed) = 4.8
    float nsp1 = 5.39f; // 13-14, int16, nsp1 (value / 100) (Maybe nsp stands for "node speed"?) = 4.25
    float nsp2 = 0.4f; // 15-16, int16, nsp2 (value / 100) = 0.5
    float nsp3 = 14.0f; // 17-18, int16, nsp3 (value / 100) = 12
    float snake_ang_speed = 0.033f; // 19-20, int16, mamu (value / 1E3) (basic snake angular speed) = 0.033
    float prey_ang_speed = 0.028f; // 21-22, int16, manu2 (value / 1E3) (angle in rad per 8ms at which prey can turn) = 0.028
    float snake_tail_k = 0.43f; // 23-24, int16, cst (value / 1E3) (snake tail speed ratio ) = 0.43
    uint8_t protocol_version = 8; // 25, int8, protocol_version = Unknown

    size_t get_size() const noexcept { return 26; }
};

std::ostream& operator<<(std::ostream & out, const packet_init & p);

#endif //SLITHER_PACKET_INIT_HPP
