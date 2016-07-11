#include "packet/p_rotation.h"

out_packet_t packet_rotation::get_rot_type() const noexcept {
  if (wang == -1.0f) {
    if (snakeSpeed == -1.0f) {
      return packet_t_rot_ccw_ang;
    } else if (ang == -1.0f) {
      return packet_t_rot_ccw_sp;
    } else {
      return packet_t_rot_ccw_ang_sp;
    }
  } else {
    if (ang == -1.0f) {
      // could use last snake direction
      if (snakeSpeed == -1.0f) {
        return packet_t_rot_ccw_wang;
      } else {
        return packet_t_rot_ccw_wang_sp;
      }
    } else {
      if (snakeSpeed == -1.0f) {
        if (is_clockwise()) {
          return packet_t_rot_cw_ang_wang;
        } else {
          return packet_t_rot_ccw_ang_wang;
        }
      } else {
        if (is_clockwise()) {
          return packet_t_rot_cw_ang_wang_sp;
        } else {
          return packet_t_rot_ccw_ang_wang_sp;
        }
      }
    }
  }
}

bool packet_rotation::is_clockwise() const noexcept {
  float dAngle = packet_rotation::normalize_angle(wang - ang);

  if (dAngle > f_pi) {
    dAngle -= f_2pi;
  }

  return dAngle > 0;
}

std::ostream& operator<<(std::ostream& out, const packet_rotation& p) {
  out << PacketBase(p.get_rot_type(), p.client_time);
  out << write_uint16(p.snakeId);

  if (p.ang != -1.0f) {
    out << write_ang8(p.ang);
  }

  if (p.wang != -1.0f) {
    out << write_ang8(p.wang);
  }

  if (p.snakeSpeed != -1.0f) {
    out << write_uint8(static_cast<uint8_t>(p.snakeSpeed * 18.0f));
  }

  return out;
}
