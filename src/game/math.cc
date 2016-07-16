#include "game/math.h"

bool Math::intersect_segments(float p0_x, float p0_y, float p1_x, float p1_y,
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

  // TODO(john.koepi): check is it better to have 2 more mul, then 1 branch
  const float s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
  if (s < 0 || s > d) {
    return false;
  }

  const float t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
  return !(t < 0 || t > d);
}

// line vw, and point p
// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float Math::distance_squared(float v_x, float v_y, float w_x, float w_y, float p_x, float p_y) {
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
float Math::distance_squared(float p0_x, float p0_y, float p1_x, float p1_y) {
  const float dx = p0_x - p1_x;
  const float dy = p0_y - p1_y;
  return dx * dx + dy * dy;
}

// points p0, p1
int32_t Math::distance_squared(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y) {
  const int32_t dx = p0_x - p1_x;
  const int32_t dy = p0_y - p1_y;
  return dx * dx + dy * dy;
}

