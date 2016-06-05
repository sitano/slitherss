#ifndef SLITHER_PACKET_FORMAT_H
#define SLITHER_PACKET_FORMAT_H

#include <ostream>
#include <cstdint>

#define M_2PI (2.0 * 3.14159265358979323846) /* 2 * pi */

typedef uint32_t uint24_t;
typedef float fixed_point_t;

template<typename _T> struct ostream_write_value { _T v; };

template<typename _T> std::ostream& operator<<(std::ostream& __os, ostream_write_value<_T> __f);

std::ostream& operator<<(std::ostream& __os, ostream_write_value<uint8_t> __f);
std::ostream& operator<<(std::ostream& __os, ostream_write_value<uint16_t> __f);
std::ostream& operator<<(std::ostream& __os, ostream_write_value<uint24_t> __f);
std::ostream& operator<<(std::ostream& __os, ostream_write_value<const std::string &> __f);

inline ostream_write_value<uint8_t> write_uint8(uint8_t v) {
    return { v };
}

inline ostream_write_value<uint16_t> write_uint16(uint16_t v) {
    return { v };
}

inline ostream_write_value<uint24_t> write_uint24(uint24_t v) {
    return { v };
}

inline ostream_write_value<uint8_t> write_fp8(fixed_point_t v) {
    return { (uint8_t)(10.0f * v /* + 0.5f */) };
}

template <size_t digits> ostream_write_value<uint16_t> write_fp16(fixed_point_t v);

template <> ostream_write_value<uint16_t> write_fp16<2>(fixed_point_t v);
template <> ostream_write_value<uint16_t> write_fp16<3>(fixed_point_t v);

inline ostream_write_value<uint8_t> write_ang8(fixed_point_t v) {
    return { (uint8_t)(256 * v / M_2PI) };
}

ostream_write_value<uint24_t> write_fp24(fixed_point_t v);
ostream_write_value<const std::string &> write_string(const std::string &s);

#endif //SLITHER_PACKET_FORMAT_H
