#include "d_clean.hpp"

std::ostream& operator<<(std::ostream & out, const packet_debug_reset & p) {
    return out << static_cast<packet_base>(p);
}

