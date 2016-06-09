#include "p_base.hpp"

std::ostream& operator<<(std::ostream & out, const packet_base & p) {
    return out << write_uint16(p.client_time) << write_uint8(p.packet_type);
}

std::istream& operator>>(std::istream & in, in_packet_t & p) {
    return in.get((std::basic_istream<char>::char_type &)p);
}
