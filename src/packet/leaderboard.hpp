#ifndef SLITHER_PACKET_LEADERBOARD_HPP
#define SLITHER_PACKET_LEADERBOARD_HPP

#include "base.hpp"
#include "game/snake.hpp"

#include <memory>
#include <vector>

struct packet_leaderboard : public packet_base {
    packet_leaderboard() : packet_base(packet_t_leaderboard) {}

    // local players rank in leaderboard (0 means not in leaderboard,
    // otherwise this is equal to the "local players rank".
    // Actually always redundant information)
    // 3, int8
    uint8_t leaderboard_rank;
    uint16_t local_rank; // 4-5, int16, local players rank
    uint16_t players; // 6-7, int16, players on server count

    /*
    ?-?	int16	parts (for snake length calculation)
    ?-?	int24	fullness (for snake length calculation; value / 16777215)
    ?-?	int8	font color (between 0 and 8)
    ?-?	int8	username length
    ?-?	string	username
    */
    std::vector<std::shared_ptr<snake>> top; // 2 + 3 + 1 + 1 string each

    size_t get_size() {
        size_t size = 8;

        for (const auto &ptr : top) {
            size += 2 + 3 + 1 + 1 + ptr->name.length();
        }

        return size;
    }
};

std::ostream& operator<<(std::ostream & out, const packet_leaderboard & p) {
    out << static_cast<packet_base>(p);
    out << write_uint8(p.leaderboard_rank);
    out << write_uint16(p.local_rank);
    out << write_uint16(p.players);
    for (const auto &ptr : p.top) {
        out << write_uint16(ptr->parts.size());
        out << write_fp24(ptr->fullness);
        out << write_uint8(ptr->color);
        out << write_string(ptr->name);
    }
    return out;
}

#endif //SLITHER_PACKET_LEADERBOARD_HPP
