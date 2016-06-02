#include "p_remove_part.hpp"

std::ostream& operator<<(std::ostream & out, const packet_remove_part & p) {
    out << static_cast<packet_base>(p);
    out << write_uint16(p.snakeId);
    if (p.fullness != 0.0f) {
        out << write_fp24(p.fullness);
    }
    return out;
}
