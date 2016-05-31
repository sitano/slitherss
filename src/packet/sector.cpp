#include "sector.hpp"

std::ostream& operator<<(std::ostream & out, const packet_sector & p) {
    out << static_cast<packet_base>(p);
    out << write_uint8(p.x);
    out << write_uint8(p.y);
    return out;
}
