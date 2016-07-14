#ifndef SRC_GAME_MATH_H_
#define SRC_GAME_MATH_H_

#include <cmath>
#include <cstdint>

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
  static bool intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
                                 float p2_x, float p2_y, float p3_x, float p3_y);

  // line vw, and point p
  // http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
  static float distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x, float p_y);

  // points p0, p1
  static float distance_squared(float p0_x, float p0_y, float p1_x, float p1_y);

  // points p0, p1
  static int32_t distance_squared(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y);

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

#endif  // SRC_GAME_MATH_H_
