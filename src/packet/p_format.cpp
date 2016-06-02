#include "p_format.hpp"

std::ostream& operator<<(std::ostream& __os, ostream_write_value<uint8_t> __f) {
    return __os.put(__f.v);
}

std::ostream& operator<<(std::ostream& __os, ostream_write_value<uint16_t> __f) {
    return __os.put((char)(__f.v >> 8)).put((char)__f.v);
}

std::ostream& operator<<(std::ostream& __os, ostream_write_value<uint24_t> __f) {
    return __os.put((char)(__f.v >> 16)).put((char)(__f.v >> 8)).put((char)__f.v);
}

std::ostream& operator<<(std::ostream& __os, ostream_write_value<const std::string &> __f) {
    __os.put((char)__f.v.length());
    for (const char c : __f.v) {
        __os.put(c);
    }
    return __os;
}

template <> ostream_write_value<uint16_t> write_fp16<2>(fixed_point_t v) {
    return { (uint16_t)(100.0f * v /* + 0.5f */) };
}

template <> ostream_write_value<uint16_t> write_fp16<3>(fixed_point_t v) {
    return { (uint16_t)(1000.0f * v /* + 0.5f */) };
}

ostream_write_value<uint24_t> write_fp24(fixed_point_t v) {
    return { (uint24_t)(v * 0xFFFFFF /* + 0.5f */) };
}

ostream_write_value<const std::string &> write_string(const std::string &s) {
    return { s };
}