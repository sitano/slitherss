#include "p_rotation.hpp"

std::ostream& operator<<(std::ostream & out, const packet_rotation & p) {
    out << packet_base(p.get_rot_type(), p.client_time);
    out << write_uint16(p.snakeId);

    if (p.ang != 0.0f) {
        out << write_ang8(p.wang);
    }

    if (p.wang != 0.0f) {
        out << write_ang8(p.wang);
    }

    if (p.snakeSpeed != 0.0f) {
        out << write_uint8(p.snakeSpeed * 18);
    }

    return out;
}

