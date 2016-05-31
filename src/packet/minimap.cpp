#include "minimap.hpp"

std::ostream& operator<<(std::ostream & out, const packet_minimap & p) {
    out << static_cast<packet_base>(p);
    for (const uint8_t v: p.data) {
        out.put(v);
    }
    return out;
}
