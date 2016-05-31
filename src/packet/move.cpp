#include "move.hpp"

std::ostream& operator<<(std::ostream & out, const packet_move & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint16(p.x);
    out << write_uint16(p.y);
    return out;
}

std::ostream& operator<<(std::ostream & out, const packet_move_rel & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint8(p.x);
    out << write_uint8(p.y);
    return out;
}

