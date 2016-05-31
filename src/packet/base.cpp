#include "base.hpp"

std::ostream& operator<<(std::ostream & out, const packet_base & p) {
    return out << write_uint16(p.client_time) << write_uint8(p.packet_type);
}
