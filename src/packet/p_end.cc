#include "p_end.h"

std::ostream& operator<<(std::ostream & out, const packet_end & p) {
    out << static_cast<packet_base>(p);
    out << write_uint8(p.status);
    return out;
}

std::ostream& operator<<(std::ostream & out, const packet_kill & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_uint24(p.kills);
    return out;
}
