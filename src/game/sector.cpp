#include "sector.hpp"

#include <algorithm>

bool intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
                        float p2_x, float p2_y, float p3_x, float p3_y) {
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    const float d = (-s2_x * s1_y + s1_x * s2_y);
    static const float epsilon = 0.0001f;
    if (d <= epsilon && d >= -epsilon) {
        return false;
    }

    // todo check is it better to have 2 more mul, then 1 branch
    const float s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
    if (s < 0 || s > d) {
        return false;
    }

    const float t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
    if (t < 0 || t > d) {
        return false;
    }

    return true;
}

// center, point, radius
bool intersect_circle(float c_x, float c_y, float p_x, float p_y, float r) {
    return distance_squared(c_x, c_y, p_x, p_y) <= r * r;
}

// line vw, and point p
// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x, float p_y) {
    // Return minimum distance between line segment vw and point p
    const float l2 = distance_squared(v_x, v_y, w_x, w_y);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 == 0.0) return distance_squared(p_x, p_y, w_x, w_x);   // v == w case
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    float t = ((p_x - v_x) * (w_x - v_x) + (p_y - v_y) * (w_y - v_y)) / l2;
    t = fmaxf(0, fminf(1, t));
    return distance_squared(p_x, p_y, v_x + t * (w_x - v_x), v_y + t * (w_y - v_y) );
}

// points p0, p1
float distance_squared(float p0_x, float p0_y, float p1_x, float p1_y) {
    const float dx = p0_x - p1_x;
    const float dy = p0_y - p1_y;
    return dx * dx + dy * dy;
}

// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
float fastsqrt(float val)  {
    union
    {
        int tmp;
        float val;
    } u;
    u.val = val;
    u.tmp -= 1<<23; /* Remove last bit so 1.0 gives 1.0 */
    /* tmp is now an approximation to logbase2(val) */
    u.tmp >>= 1; /* divide by 2 */
    u.tmp += 1<<29; /* add 64 to exponent: (e+127)/2 =(e/2)+63, */
    /* that represents (e/2)-64 but we want e/2 */
    return u.val;
}

/*
float fastinvsqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i>>1);
    x = *(float*)&i;
    return x*(1.5f - xhalf*x*x);
}
*/

void snake_bb::sort() {
    std::sort(sectors.begin(), sectors.end());
}

bool snake_bb::binary_search(sector *s) {
    return std::binary_search(sectors.begin(), sectors.end(), s);
}

size_t snake_bb::get_sectors_count() {
    return sectors.size();
}

size_t snake_bb::get_snakes_in_sectors_count() {
    size_t i = 0;
    for (sector *s : sectors) {
        i += s->m_snakes.size();
    }
    return i;
}

void snake_bb::reg_new_sector_if_missing(sector *s) {
    if (std::find(new_sectors.begin(), new_sectors.end(), s) == new_sectors.end()) {
        new_sectors.push_back(s);
    }
}

void snake_bb::reg_old_sector_if_missing(sector *s) {
    if (std::find(old_sectors.begin(), old_sectors.end(), s) == old_sectors.end()) {
        old_sectors.push_back(s);
    }
}

void sector::remove_snake(snake_id_t id) {
    m_snakes.erase(std::remove_if(m_snakes.begin(), m_snakes.end(), [id](const snake_bb &bb){ return bb.id == id; }));
}

void sectors::init_sectors(const uint16_t sector_count_along_edge) {
    width = sector_count_along_edge;
    const size_t len = sector_count_along_edge * sector_count_along_edge;
    reserve(len);
    for (size_t i = 0; i < len; i ++) {
        push_back(sector{
                static_cast<uint8_t>(i % sector_count_along_edge),
                static_cast<uint8_t>(i / sector_count_along_edge),
                {}, {}
        });
    }
}

size_t sectors::get_index(uint16_t x, uint16_t y) {
    return y * width + x;
}

sector *sectors::get_sector(uint16_t x, uint16_t y) {
    return &operator[](get_index(x, y));
}



