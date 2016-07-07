#ifndef SRC_PACKET_BASE_H_
#define SRC_PACKET_BASE_H_

#include <iostream>
#include "p_format.h"

enum out_packet_t : uint8_t;

struct packet_base {
  uint16_t client_time = 0;  // 2 bytes - time since last message from client
  out_packet_t packet_type;  // 1 byte - packet type

  packet_base() = default;
  explicit packet_base(out_packet_t t) : packet_type(t) {}
  packet_base(out_packet_t t, uint16_t clock)
      : client_time(clock), packet_type(t) {}

  size_t get_size() const noexcept { return 3; }
};

enum in_packet_t : uint8_t {
  in_packet_t_angle = 0,   // 0 - 250 mouseMove: the input angle.
                           // Counter-clockwise, (0 and 250 point right, 62
                           // points up)
  in_packet_t_ping = 251,  // Pings the server. Sent every 250ms, but not before
                           // a pong has been received after the last ping.
  in_packet_t_rot_left = 108,   // keyDown, keyUp (left-arrow or right-arrow):
                                // start/stop turning left or right
  in_packet_t_rot_right = 114,  // keyDown, keyUp (left-arrow or right-arrow):
                                // start/stop turning left or right
  in_packet_t_start_acc = 253,  // mouseDown, keyDown (space or up-arrow): the
                                // snake is entering speed mode
  in_packet_t_stop_acc = 254,   // mouseUp, keyUp (space or up-arrow): the snake
                                // is leaving speed mode
  in_packet_t_username_skin = 's',    // 115, Packet SetUsernameAndSkin
  in_packet_t_victory_message = 255,  // Packet SaveVictoryMessage
};

enum out_packet_t : uint8_t {
  packet_t_init = 'a',  // Initial setup
  packet_t_rot_ccw_wang_sp =
      'E',  // Snake rotation counterclockwise (dir wang ?sp)
  packet_t_rot_ccw_wang =
      'E',  // Snake rotation counterclockwise (dir wang ?sp)
  packet_t_rot_ccw_ang_wang =
      '3',  // Snake rotation counterclockwise (dir ang wang | sp)
  packet_t_rot_ccw_sp =
      '3',  // Snake rotation counterclockwise (dir ang wang | sp)
  packet_t_rot_ccw_ang_wang_sp =
      'e',  // Snake rotation counterclockwise (?dir ang ?wang ?sp)
  packet_t_rot_ccw_ang_sp =
      'e',  // Snake rotation counterclockwise (?dir ang ?wang ?sp)
  packet_t_rot_ccw_ang =
      'e',  // Snake rotation counterclockwise (?dir ang ?wang ?sp)
  packet_t_rot_cw_ang_wang_sp =
      '4',  // Snake rotation clockwise (dir ang? wang ?sp)
  packet_t_rot_cw_wang_sp =
      '4',                     // Snake rotation clockwise (dir ang? wang ?sp)
  packet_t_rot_cw_wang = '4',  // Snake rotation clockwise (dir ang? wang ?sp)
  packet_t_rot_cw_ang_wang = '5',  // Snake rotation clockwise (dir ang wang)
  packet_t_set_fullness = 'h',     // Update snake last body part fullness (fam)
  packet_t_rem_part = 'r',         // Remove snake part
  packet_t_mov = 'g',              // Move snake
  packet_t_mov_rel = 'G',          // Move snake
  packet_t_inc = 'n',              // Increase snake
  packet_t_inc_rel = 'N',          // Increase snake
  packet_t_leaderboard = 'l',      // Leaderboard
  packet_t_end = 'v',              // dead/disconnect packet
  packet_t_add_sector = 'W',       // Add Sector
  packet_t_rem_sector = 'w',       // Remove Sector
  packet_t_highscore = 'm',        // Global highscore
  packet_t_pong = 'p',             // Pong
  packet_t_minimap = 'u',          // Update minimap
  packet_t_snake = 's',            // Add/remove Snake
  packet_t_set_food =
      'F',  // Add Food, Sent when food that existed before enters range.
  packet_t_spawn_food = 'b',  // Add Food, Sent when food is created because of
                              // turbo or the death of a snake.
  packet_t_add_food =
      'f',  // Add Food, Sent when natural food spawns while in range.
  packet_t_eat_food = 'c',  // Food eaten
  packet_t_mov_prey = 'j',  // Update Prey
  packet_t_add_prey = 'y',  // Add/remove Prey
  packet_t_rem_prey = 'y',  // Add/remove Prey
  packet_t_kill = 'k',      // Kill (unused in the game-code)

  // custom debug
  packet_d_reset = '0',  // reset debug render buffer
  packet_d_draw = '!',   // draw something
};

std::ostream& operator<<(std::ostream& out, const packet_base& p);
std::istream& operator>>(std::istream& in, in_packet_t& p);

#endif  // SRC_PACKET_BASE_H_
