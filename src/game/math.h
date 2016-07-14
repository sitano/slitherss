#ifndef SRC_SERVER_MATH_H_
#define SRC_SERVER_MATH_H_

#include "math.h"

struct Point {
  float x;
  float y;
};

struct Circle : Point {
  float r;
};

struct Rect {
  Point p0, p1;
};

class Math {
  Math() = delete;

 public:
  static constexpr float f_pi = 3.14159265358979323846f;
  static constexpr float f_2pi = 2.0f * f_pi;

  inline static float normalize_angle(float ang) {
    return ang - f_2pi * floorf(ang / f_2pi);
  }

  /**
   * http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
   *
   * FWIW, the following function (in C) both detects line intersections and determines the intersection point.
   * It is based on an algorithm in Andre LeMothe's "Tricks of the Windows Game Programming Gurus".
   * It's not dissimilar to some of the algorithm's in other answers (e.g. Gareth's).
   * LeMothe then uses Cramer's Rule (don't ask me) to solve the equations themselves.
   *
   * I can attest that it works in my feeble asteroids clone, and seems to deal correctly with the edge cases
   * described in other answers by Elemental, Dan and Wodzu. It's also probably faster than the code posted by
   * KingNestor because it's all multiplication and division, no square roots!
   */
  inline static bool intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
                                        float p2_x, float p2_y, float p3_x, float p3_y) {
    const float s1_x = p1_x - p0_x;
    const float s1_y = p1_y - p0_y;
    const float s2_x = p3_x - p2_x;
    const float s2_y = p3_y - p2_y;

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

    const float t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
    return !(t < 0 || t > d);
  }

  // line vw, and point p
  // http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
  inline static float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x, float p_y) {
    // Return minimum distance between line segment vw and point p
    const float l2 = distance_squared(v_x, v_y, w_x, w_y);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 == 0.0) {  // v == w case
      return distance_squared(p_x, p_y, w_x, w_x);
    }
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    float t = ((p_x - v_x) * (w_x - v_x) + (p_y - v_y) * (w_y - v_y)) / l2;
    t = fmaxf(0, fminf(1, t));
    return distance_squared(p_x, p_y, v_x + t * (w_x - v_x), v_y + t * (w_y - v_y));
  }

  // points p0, p1
  inline static float distance_squared(float p0_x, float p0_y, float p1_x, float p1_y) {
    const float dx = p0_x - p1_x;
    const float dy = p0_y - p1_y;
    return dx * dx + dy * dy;
  }

  // points p0, p1
  inline static int32_t distance_squared(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y) {
    const int32_t dx = p0_x - p1_x;
    const int32_t dy = p0_y - p1_y;
    return dx * dx + dy * dy;
  }

  // center, point, radius
  inline static bool intersect_circle(float c_x, float c_y, float p_x, float p_y, float r) {
    return distance_squared(c_x, c_y, p_x, p_y) <= r * r;
  }

  // https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
  inline static float fast_sqrt(float val) {
    union {
      int tmp;
      float val;
    } u;

    u.val = val;
    u.tmp -= 1 << 23; /* Remove last bit so 1.0 gives 1.0 */
    /* tmp is now an approximation to logbase2(val) */
    u.tmp >>= 1;      /* divide by 2 */
    u.tmp += 1 << 29; /* add 64 to exponent: (e+127)/2 =(e/2)+63, */
    /* that represents (e/2)-64 but we want e/2 */

    return u.val;
  }

  // https://en.wikipedia.org/wiki/Fast_inverse_square_root
  // https://betterexplained.com/articles/understanding-quakes-fast-inverse-square-root/
  inline static float fast_inv_sqrt(float x) {
    union {
      int tmp;
      float val;
    } u;

    const float xhalf = 0.5f * x;
    u.val = x;
    u.tmp = 0x5f3759df - (u.tmp >> 1);
    return u.val * (1.5f - xhalf * u.val * u.val);
  }

};

#endif  // SRC_SERVER_MATH_H_
