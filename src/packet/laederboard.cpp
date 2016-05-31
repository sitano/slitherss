#include "leaderboard.hpp"

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

