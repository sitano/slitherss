#ifndef SLITHER_PACKET_LEADERBOARD_HPP
#define SLITHER_PACKET_LEADERBOARD_HPP

#include "game/snake.h"
#include "p_base.h"

#include <memory>
#include <vector>

struct packet_leaderboard : public packet_base {
  packet_leaderboard() : packet_base(packet_t_leaderboard) {}

  // local players rank in leaderboard (0 means not in leaderboard,
  // otherwise this is equal to the "local players rank".
  // Actually always redundant information)
  // 3, int8
  uint8_t leaderboard_rank = 0;
  uint16_t local_rank = 0;  // 4-5, int16, local players rank
  uint16_t players = 0;     // 6-7, int16, players on server count

  /*
  ?-?	int16	parts (for snake length calculation)
  ?-?	int24	fullness (for snake length calculation; value / 16777215)
  ?-?	int8	font color (between 0 and 8)
  ?-?	int8	username length
  ?-?	string	username
  */
  std::vector<std::shared_ptr<snake>> top;  // 2 + 3 + 1 + 1 string each

  size_t get_size() const noexcept {
    size_t size = 8;

    for (const auto& ptr : top) {
      size += 2 + 3 + 1 + 1 + ptr->name.length();
    }

    return size;
  }
};

std::ostream& operator<<(std::ostream& out, const packet_leaderboard& p);

#endif  // SLITHER_PACKET_LEADERBOARD_HPP
