#include "d_clean.h"

std::ostream& operator<<(std::ostream & out, const packet_debug_reset & p) {
    return out << static_cast<packet_base>(p);
}

