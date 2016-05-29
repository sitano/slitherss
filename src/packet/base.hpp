#ifndef SLITHER_SERVER_BASE_HPP
#define SLITHER_SERVER_BASE_HPP

#include "format.hpp"

#define packet_t_none (packet_t)0

enum packet_t : uint8_t;

struct packet_base {
    uint16_t client_time = 0; // 2 bytes - time since last message from client
    packet_t packet_type = packet_t_none; // 1 byte - packet type

    packet_base() = default;
    packet_base(packet_t t) : packet_type(t) {}
    packet_base(packet_t t, uint16_t clock) : client_time(clock), packet_type(t) {}

    size_t get_size() { return 3; }
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
    packet_t_leaderboard = 'l', // Leaderboard
    packet_t_end = 'v', // dead/disconnect packet
    packet_t_add_sector = 'W', // Add Sector
    packet_t_rem_sector = 'w', // Remove Sector
    packet_t_highscore = 'm', // Global highscore
    packet_t_pong = 'p', // Pong
    packet_t_minimap = 'u', // Update minimap
    packet_t_snake = 's', // Add/remove Snake
    packet_t_set_food = 'F', // Add Food, Sent when food that existed before enters range.
    packet_t_spawn_food = 'b', // Add Food, Sent when food is created because of turbo or the death of a snake.
    packet_t_add_food = 'f', // Add Food, Sent when natural food spawns while in range.
    packet_t_eat_food = 'c', // Food eaten
    packet_t_mov_prey = 'j', // Update Prey
    packet_t_add_prey = 'y', // Add/remove Prey
    packet_t_rem_prey = 'y', // Add/remove Prey
    packet_t_kill = 'k', // Kill (unused in the game-code)
};

std::ostream& operator<<(std::ostream & out, const packet_base & p) {
    return out << write_uint16(p.client_time) << write_uint8(p.packet_type);
}

#endif //SLITHER_SERVER_BASE_HPP
