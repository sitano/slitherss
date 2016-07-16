#ifndef SRC_PACKET_P_MINIMAP_H_
#define SRC_PACKET_P_MINIMAP_H_

#include "packet/p_base.h"

#include <vector>

struct packet_minimap : public PacketBase {
  packet_minimap() : PacketBase(packet_t_minimap) {}

  /**
   *
   * The minimap has a size of 80*80
   * Start at the top-left, go to the right, when at the right, repeat for the
   * next row and so on
   * Start reading the packet at index 3
   * Read one byte:
   * value >= 128: skip (value - 128) pixels
   * value < 128: repeat for every bit (from the 64-bit to the 1-bit):
   * if set, paint the current pixel
   * go to the next pixel
   *
   */

  std::vector<uint8_t> data;

  size_t get_size() const noexcept { return 3 + data.size(); }
};

std::ostream& operator<<(std::ostream& out, const packet_minimap& p);

#endif  // SRC_PACKET_P_MINIMAP_H_
