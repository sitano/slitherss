#ifndef SLITHER_SERVER_BASE_HPP
#define SLITHER_SERVER_BASE_HPP

#include "format.hpp"
#include "buf.hpp"

#define packet_t_none (packet_t)0

enum packet_t : uint8_t;

struct packet_base {
    uint16_t client_time = 0; // 2 bytes - time since last message from client
    packet_t packet_type = packet_t_none; // 1 byte - packet type

    typedef packet_size<3> size;

    packet_base() = default;
    packet_base(packet_t t) : packet_type(t) {}
    packet_base(packet_t t, uint16_t clock) : client_time(clock), packet_type(t) {}
};

enum packet_t : uint8_t {
    packet_t_init = 'a', // Initial setup
    packet_t_rot_ccw_wang_sp = 'E', // Snake rotation counterclockwise (dir wang ?sp)
    packet_t_rot_ccw_wang = 'E', // Snake rotation counterclockwise (dir wang ?sp)
    packet_t_rot_ccw_ang_wang = '3', // Snake rotation counterclockwise (dir ang wang | sp)
    packet_t_rot_ccw_sp = '3', // Snake rotation counterclockwise (dir ang wang | sp)
    packet_t_rot_ccw_ang_wang_sp = 'e', // Snake rotation counterclockwise (?dir ang ?wang ?sp)
    packet_t_rot_ccw_ang_sp = 'e', // Snake rotation counterclockwise (?dir ang ?wang ?sp)
    packet_t_rot_ccw_ang = 'e', // Snake rotation counterclockwise (?dir ang ?wang ?sp)
    packet_t_rot_cw_ang_wang_sp = '4', // Snake rotation clockwise (dir ang? wang ?sp)
    packet_t_rot_cw_wang_sp = '4', // Snake rotation clockwise (dir ang? wang ?sp)
    packet_t_rot_cw_wang = '4', // Snake rotation clockwise (dir ang? wang ?sp)
    packet_t_rot_cw_ang_wang = '5', // Snake rotation clockwise (dir ang wang)
    packet_t_set_fullness = 'h', // Update snake last body part fullness (fam)
    packet_t_rem_part = 'r', // Remove snake part
    packet_t_mov = 'g', // Move snake
    packet_t_mov_rel = 'G', // Move snake
    packet_t_inc = 'n', // Increase snake
    packet_t_inc_rel = 'N', // Increase snake
    packet_t_l = 'l', // Leaderboard
    packet_t_v = 'v', // dead/disconnect packet
    packet_t_W = 'W', // Add Sector
    packet_t_w = 'w', // Remove Sector
    packet_t_m = 'm', // Global highscore
    packet_t_p = 'p', // Pong
    packet_t_u = 'u', // Update minimap
    packet_t_s = 's', // Add/remove Snake
    packet_t_F = 'F', // Add Food
    packet_t_b = 'b', // Add Food
    packet_t_f = 'f', // Add Food
    packet_t_c = 'c', // Food eaten
    packet_t_j = 'j', // Update Prey
    packet_t_y = 'y', // Add/remove Prey
    packet_t_k = 'k', // Kill (unused in the game-code)
};

std::ostream& operator<<(std::ostream & out, const packet_base & p) {
    return out << write_uint16(p.client_time) << write_uint8(p.packet_type);
}

#endif //SLITHER_SERVER_BASE_HPP
