#include "server/game.h"

#include <algorithm>

GameServer::GameServer() {
  // set up access channels to only log interesting things
  endpoint.clear_access_channels(alevel::all);
  endpoint.set_access_channels(alevel::access_core);
  endpoint.set_access_channels(alevel::app);

  // Initialize the Asio transport policy
  endpoint.init_asio();
  endpoint.set_reuse_addr(true);

  // Bind the handlers we are using
  endpoint.set_socket_init_handler(bind(&GameServer::on_socket_init, this, ::_1, ::_2));

  endpoint.set_open_handler(bind(&GameServer::on_open, this, _1));
  endpoint.set_message_handler(bind(&GameServer::on_message, this, _1, _2));
  endpoint.set_close_handler(bind(&GameServer::on_close, this, _1));
}

int GameServer::Run(IncomingConfig config) {
  endpoint.get_alog().write(alevel::app,
      "Running slither server on port " + std::to_string(config.port));

  m_config = config;
  PrintWorldInfo();

  endpoint.listen(config.port);
  endpoint.start_accept();

  m_world.init(config.world);
  m_init = BuildInitPacket();
  NextTick(GetCurrentTime());

  try {
    endpoint.get_alog().write(alevel::app, "Server started...");
    endpoint.run();
    return 0;
  } catch (websocketpp::exception const &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
}

void GameServer::PrintWorldInfo() {
  std::stringstream s;
  s << "World info = \n" << m_world;
  endpoint.get_alog().write(alevel::app, s.str());
}

void GameServer::NextTick(long last) {
  last_time_point = last;
  timer = endpoint.set_timer(
      std::max(0L, timer_interval_ms - (GetCurrentTime() - last)),
      bind(&GameServer::on_timer, this, _1));
}

void GameServer::on_timer(error_code const &ec) {
  const long now = GetCurrentTime();
  const long dt = now - last_time_point;

  if (ec) {
    endpoint.get_alog().write(alevel::app, "Main game loop timer error: " + ec.message());
    return;
  }

  m_world.tick(dt);
  BroadcastDebug();
  BroadcastUpdates();
  RemoveDeadSnakes();

  const long step_time = GetCurrentTime() - now;
  if (step_time > 10) {
    endpoint.get_alog().write(alevel::app,
        "Load is too high, step took " + std::to_string(step_time) + "ms");
  }

  NextTick(now);
}

void GameServer::BroadcastDebug() {
  if (!m_config.debug) {
    return;
  }

  packet_debug_draw draw;

  for (snake *s : m_world.get_changes()) {
    uint16_t sis = static_cast<uint16_t>(s->id * 1000);

    // bound box
    draw.circles.push_back(
        d_draw_circle{sis++, {s->sbb.x, s->sbb.y}, s->sbb.r, 0xc8c8c8});

    // body inner circles
    const float r1 = s->get_snake_body_part_radius();

    draw.circles.push_back(
        d_draw_circle{sis++, {s->get_head_x(), s->get_head_y()}, r1, 0xc80000});

    const body &sec = *(s->parts.begin() + 1);
    draw.circles.push_back(d_draw_circle{sis++, {sec.x, sec.y}, r1, 0x3c3c3c});
    draw.circles.push_back(
        d_draw_circle{sis++,
                      {sec.x + (s->get_head_x() - sec.x) / 2.0f,
                       sec.y + (s->get_head_y() - sec.y) / 2.0f},
                      r1,
                      0x646464});
    draw.circles.push_back(d_draw_circle{
        sis++, {s->parts.back().x, s->parts.back().y}, r1, 0x646464});

    // bounds
    for (const sector *ss : s->sbb.m_sectors) {
      draw.circles.push_back(
          d_draw_circle{sis++, {ss->box.x, ss->box.y}, ss->box.r, 0x511883});
    }

    // intersection algorithm
    static const size_t head_size = 8;
    static const size_t tail_step = static_cast<size_t>(
        WorldConfig::sector_size / snake::tail_step_distance);
    static const size_t tail_step_half = tail_step / 2;
    const size_t len = s->parts.size();

    if (len <= head_size + tail_step) {
      for (const body &b : s->parts) {
        draw.circles.push_back(d_draw_circle{
            sis++, {b.x, b.y}, WorldConfig::move_step_distance, 0x646464});
      }
    } else {
      auto p = s->parts[3];
      draw.circles.push_back(d_draw_circle{
          sis++, {p.x, p.y}, WorldConfig::sector_size / 2, 0x848484});
      p = s->parts[0];
      draw.circles.push_back(d_draw_circle{
          sis++, {p.x, p.y}, WorldConfig::move_step_distance, 0x646464});
      p = s->parts[8];
      draw.circles.push_back(d_draw_circle{
          sis++, {p.x, p.y}, WorldConfig::move_step_distance, 0x646464});

      auto end = s->parts.end();
      for (auto i = s->parts.begin() + 7 + tail_step_half; i < end;
           i += tail_step) {
        draw.circles.push_back(d_draw_circle{
            sis++, {i->x, i->y}, WorldConfig::sector_size / 2, 0x848484});
      }
    }
  }

  if (!draw.empty()) {
    broadcast_debug(draw);
  }
}

void GameServer::BroadcastUpdates() {
  for (auto ptr : m_world.get_changes()) {
    const snake_id_t id = ptr->id;
    const uint8_t flags = ptr->update;

    if (flags & change_dead) {
      continue;
    }

    if (flags & change_dying) {
      endpoint.get_alog().write(alevel::app,
        "Found dying snake " + std::to_string(id));

      if (!ptr->bot) {
        const auto ses_i = LoadSessionIter(id);
        if (ses_i != m_sessions.end()) {
          send_binary(ses_i, packet_end(packet_end::status_death));
        }
      }

      ptr->spawn_food_when_dead(&m_world.get_sectors(), [&]() -> float {
        return m_world.next_randomf();
      });
      SendFoodUpdate(ptr);

      broadcast_binary(packet_remove_snake(ptr->id, packet_remove_snake::status_snake_died));
      broadcast_binary(packet_remove_snake(ptr->id, packet_remove_snake::status_snake_left));

      ptr->update |= change_dead;

      if (ptr->bot) {
        m_world.get_dead().push_back(ptr->id);
      }

      continue;
    }

    if (flags) {
      if (flags & (change_angle | change_speed)) {
        packet_rotation rot = packet_rotation();
        rot.snakeId = id;

        if (flags & change_angle) {
          ptr->update ^= change_angle;
          rot.ang = ptr->angle;

          if (flags & change_wangle) {
            ptr->update ^= change_wangle;
            rot.wang = ptr->wangle;
          }
        }

        if (flags & change_speed) {
          ptr->update ^= change_speed;
          rot.snakeSpeed = ptr->speed / 32.0f;
        }

        broadcast_binary(rot);
      }

      if (flags & change_pos) {
        ptr->update ^= change_pos;

        // increase length
        if (ptr->clientPartsIndex < ptr->parts.size()) {
          broadcast_binary(packet_inc(ptr));
          ptr->clientPartsIndex++;
        } else {
          // decrease length
          if (ptr->clientPartsIndex > ptr->parts.size()) {
            broadcast_binary(packet_remove_part(ptr));
            ptr->clientPartsIndex--;
          }

          // move
          broadcast_binary(packet_move(ptr));
        }

        SendFoodUpdate(ptr);
        if (!ptr->bot) {
          const auto ses_i = LoadSessionIter(id);
          SendPOVUpdateTo(ses_i, ptr);

          if (flags & change_fullness) {
            send_binary(ses_i, packet_fullness(ptr));
            ptr->update ^= change_fullness;
          }
        }
      }
    }
  }

  m_world.flush_changes();
}

void GameServer::SendPOVUpdateTo(sessions::iterator ses_i, snake *ptr) {
  if (!ptr->vp.new_sectors.empty()) {
    for (const sector *s_ptr : ptr->vp.new_sectors) {
      send_binary(ses_i, packet_add_sector(s_ptr->x, s_ptr->y));
      send_binary(ses_i, packet_set_food(&s_ptr->m_food));
    }
    ptr->vp.new_sectors.clear();
  }

  if (!ptr->vp.old_sectors.empty()) {
    for (const sector *s_ptr : ptr->vp.old_sectors) {
      send_binary(ses_i, packet_remove_sector(s_ptr->x, s_ptr->y));
    }
    ptr->vp.old_sectors.clear();
  }
}

void GameServer::SendFoodUpdate(snake *ptr) {
  if (!ptr->eaten.empty()) {
    const snake_id_t id = ptr->id;
    for (const Food &f : ptr->eaten) {
      // todo: to those who observers me
      broadcast_binary(packet_eat_food(id, f));
    }
    ptr->eaten.clear();
  }

  if (!ptr->spawn.empty()) {
    for (const Food &f : ptr->spawn) {
      // todo: to those who observers me
      broadcast_binary(packet_spawn_food(f));
    }
    ptr->spawn.clear();
  }
}

void GameServer::RemoveDeadSnakes() {
  for (auto id : m_world.get_dead()) {
    RemoveSnake(id);
  }

  m_world.get_dead().clear();
}

void GameServer::on_socket_init(websocketpp::connection_hdl,
                                    boost::asio::ip::tcp::socket &s) {
  boost::asio::ip::tcp::no_delay option(true);
  s.set_option(option);
}

void GameServer::on_open(connection_hdl hdl) {
  const auto new_snake_ptr = m_world.create_snake();
  m_world.add_snake(new_snake_ptr);

  m_sessions[hdl] = Session(new_snake_ptr->id, GetCurrentTime());
  m_connections[new_snake_ptr->id] = hdl;

  endpoint.send_binary(hdl, m_init);

  // send snake
  const auto ses_i = m_sessions.find(hdl);
  broadcast_binary(packet_add_snake(new_snake_ptr.get()));
  broadcast_binary(packet_move(new_snake_ptr.get()));
  SendPOVUpdateTo(ses_i, new_snake_ptr.get());

  // introduce other snakes in sectors view
  for (auto ptr : m_world.get_snakes()) {
    if (ptr.first != new_snake_ptr->id) {
      const snake *s = ptr.second.get();
      send_binary(ses_i, packet_add_snake(s));
      send_binary(ses_i, packet_move(s));
    }
  }
}

void GameServer::on_message(connection_hdl hdl, message_ptr ptr) {
  if (ptr->get_opcode() != opcode::binary) {
    endpoint.get_alog().write(
        alevel::app,
        "Unknown incoming message opcode " + std::to_string(ptr->get_opcode()));
    return;
  }

  // reader
  std::stringstream buf(ptr->get_payload(), std::ios_base::in);

  in_packet_t packet_type = in_packet_t_angle;
  buf >> packet_type;

  // len check
  const size_t len = ptr->get_payload().size();
  if (len > 255) {
    endpoint.get_alog().write(
        alevel::app, "Packet '" + std::to_string(packet_type) + "' too big " +
                         std::to_string(len));
    return;
  }

  // session obtain
  const auto ses_i = m_sessions.find(hdl);
  if (ses_i == m_sessions.end()) {
    endpoint.get_alog().write(alevel::app, "No session, skip packet");
    return;
  }

  // last client time manage
  Session &ss = ses_i->second;

  // parsing
  if (packet_type <= 250 && len == 1) {
    // in_packet_t_angle, [0 - 250]
    const float angle = world::f_pi * packet_type / 125.0f;
    DoSnake(ss.snake_id, [=](snake *s) {
      s->wangle = angle;
      s->update |= change_wangle;
    });
    return;
  }

  switch (packet_type) {
    case in_packet_t_ping:
      send_binary(ses_i, packet_pong());
      break;

    case in_packet_t_username_skin:
      buf >> ss.protocol_version;
      buf >> ss.skin;
      buf.str(ss.name);

      DoSnake(ss.snake_id, [&ss](snake *s) {
        s->name = ss.name;
        s->skin = ss.skin;
      });
      break;

    case in_packet_t_victory_message:
      buf >> packet_type;  // always 118
      buf.str(ss.message);
      break;

    case in_packet_t_rot_left:
      buf >> packet_type;  // vfrb (virtual frames count) [0 - 127] of turning
                           // into the right direction
      // snake.eang -= mamu * v * snake.scang * snake.spang)
      endpoint.get_alog().write(
          alevel::app, "rotate ccw, snake " + std::to_string(ss.snake_id) +
                           ", vfrb " + std::to_string(packet_type));
      break;

    case in_packet_t_rot_right:
      buf >> packet_type;  // vfrb (virtual frames count) [0 - 127] of turning
                           // into the right direction
      // snake.eang += mamu * v * snake.scang * snake.spang)
      endpoint.get_alog().write(
          alevel::app, "rotate cw, snake " + std::to_string(ss.snake_id) +
                           ", vfrb " + std::to_string(packet_type));
      break;

    case in_packet_t_start_acc:
      DoSnake(ss.snake_id, [](snake *s) { s->acceleration = true; });
      break;

    case in_packet_t_stop_acc:
      DoSnake(ss.snake_id, [](snake *s) { s->acceleration = false; });
      break;

    default:
      endpoint.get_alog().write(
          alevel::app, "Unknown packet type " + std::to_string(packet_type) +
                           ", len " +
                           std::to_string(ptr->get_payload().size()));
      break;
  }
}

void GameServer::on_close(connection_hdl hdl) {
  const auto ptr = m_sessions.find(hdl);
  if (ptr != m_sessions.end()) {
    const snake_id_t snakeId = ptr->second.snake_id;
    m_sessions.erase(ptr->first);
    RemoveSnake(snakeId);
  }
}

void GameServer::RemoveSnake(snake_id_t id) {
  m_connections.erase(id);
  m_world.remove_snake(id);
}

PacketInit GameServer::BuildInitPacket() {
  PacketInit init;

  init.game_radius = WorldConfig::game_radius;
  init.max_snake_parts = WorldConfig::max_snake_parts;
  init.sector_size = WorldConfig::sector_size;
  init.sector_count_along_edge = WorldConfig::sector_count_along_edge;

  init.spangdv = snake::spangdv;
  init.nsp1 = snake::nsp1;
  init.nsp2 = snake::nsp2;
  init.nsp3 = snake::nsp3;

  init.snake_ang_speed = 8.0f * snake::snake_angular_speed / 1000.0f;
  init.prey_ang_speed = 8.0f * snake::prey_angular_speed / 1000.0f;
  init.snake_tail_k = snake::snake_tail_k;

  init.protocol_version = world::protocol_version;

  return init;
}

long GameServer::GetCurrentTime() {
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;
  using std::chrono::steady_clock;

  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void GameServer::DoSnake(snake_id_t id, std::function<void(snake *)> f) {
  if (id > 0) {
    const auto snake_i = m_world.get_snake(id);
    if (snake_i->first == id) {
      f(snake_i->second.get());
    }
  }
}

GameServer::sessions::iterator GameServer::LoadSessionIter(
    snake_id_t id) {
  const auto hdl_i = m_connections.find(id);
  if (hdl_i == m_connections.end()) {
    endpoint.get_alog().write(
        websocketpp::log::alevel::app,
        "Failed to locate snake connection " + std::to_string(id));
    return m_sessions.end();
  }

  const auto ses_i = m_sessions.find(hdl_i->second);
  if (ses_i == m_sessions.end()) {
    endpoint.get_alog().write(
        websocketpp::log::alevel::app,
        "Failed to locate snake session " + std::to_string(id));
  }

  return ses_i;
}
