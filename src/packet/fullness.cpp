#include "fullness.hpp"

std::ostream& operator<<(std::ostream & out, const packet_fullness & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    out << write_fp24(p.fullness);
    return out;
}

