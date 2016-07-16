#include "server/game.h"

#include <algorithm>

#include "game/math.h"

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

int GameServer::Run(IncomingConfig in_config) {
  endpoint.get_alog().write(alevel::app, "Running slither server on port " + std::to_string(in_config.port));

  config = in_config;
  PrintWorldInfo();

  endpoint.listen(in_config.port);
  endpoint.start_accept();

  world.Init(in_config.world);
  init = BuildInitPacket();
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
  s << "World info = \n" << world;
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
    endpoint.get_alog().write(alevel::app,
        "Main game loop timer error: " + ec.message());
    return;
  }

  world.Tick(dt);
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
  if (!config.debug) {
    return;
  }

  packet_debug_draw draw;

  for (Snake *s : world.GetChangedSnakes()) {
    uint16_t sis = static_cast<uint16_t>(s->id * 1000);

    // bound box
    draw.circles.push_back(
        d_draw_circle{sis++, {s->sbb.x, s->sbb.y}, s->sbb.r, 0xc8c8c8});

    // body inner circles
    const float r1 = s->get_snake_body_part_radius();

    draw.circles.push_back(
        d_draw_circle{sis++, {s->get_head_x(), s->get_head_y()}, r1, 0xc80000});

    const Body &sec = *(s->parts.begin() + 1);
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
    for (const Sector *ss : s->sbb.sectors) {
      draw.circles.push_back(
          d_draw_circle{sis++, {ss->box.x, ss->box.y}, ss->box.r, 0x511883});
    }

    // intersection algorithm
    static const size_t head_size = 8;
    static const size_t tail_step = static_cast<size_t>(
        WorldConfig::sector_size / Snake::tail_step_distance);
    static const size_t tail_step_half = tail_step / 2;
    const size_t len = s->parts.size();

    if (len <= head_size + tail_step) {
      for (const Body &b : s->parts) {
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
      for (auto i = s->parts.begin() + 7 + tail_step_half; i < end; i += tail_step) {
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
  for (auto ptr : world.GetChangedSnakes()) {
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
        if (ses_i != sessions.end()) {
          send_binary(ses_i, packet_end(packet_end::status_death));
        }
      }

      ptr->on_dead_food_spawn(&world.GetSectors(), [&]() -> float {
        return world.NextRandomf();
      });
      SendFoodUpdate(ptr);

      broadcast_binary(packet_remove_snake(ptr->id, packet_remove_snake::status_snake_died));
      broadcast_binary(packet_remove_snake(ptr->id, packet_remove_snake::status_snake_left));

      ptr->update |= change_dead;

      if (ptr->bot) {
        world.GetDead().push_back(ptr->id);
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

  world.FlushChanges();
}

void GameServer::SendPOVUpdateTo(SessionIter ses_i, Snake *ptr) {
  if (!ptr->vp.new_sectors.empty()) {
    for (const Sector *s_ptr : ptr->vp.new_sectors) {
      send_binary(ses_i, packet_add_sector(s_ptr->x, s_ptr->y));
      send_binary(ses_i, packet_set_food(&s_ptr->food));
    }
    ptr->vp.new_sectors.clear();
  }

  if (!ptr->vp.old_sectors.empty()) {
    for (const Sector *s_ptr : ptr->vp.old_sectors) {
      send_binary(ses_i, packet_remove_sector(s_ptr->x, s_ptr->y));
    }
    ptr->vp.old_sectors.clear();
  }
}

void GameServer::SendFoodUpdate(Snake *ptr) {
  if (!ptr->eaten.empty()) {
    const snake_id_t id = ptr->id;
    for (const Food &f : ptr->eaten) {
      // TODO(john.koepi): to those who observers me
      broadcast_binary(packet_eat_food(id, f));
    }
    ptr->eaten.clear();
  }

  if (!ptr->spawn.empty()) {
    for (const Food &f : ptr->spawn) {
      // TODO(john.koepi): to those who observers me
      broadcast_binary(packet_spawn_food(f));
    }
    ptr->spawn.clear();
  }
}

void GameServer::RemoveDeadSnakes() {
  for (auto id : world.GetDead()) {
    RemoveSnake(id);
  }

  world.GetDead().clear();
}

void GameServer::on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket &s) {
  boost::asio::ip::tcp::no_delay option(true);
  s.set_option(option);
}

void GameServer::on_open(connection_hdl hdl) {
  const auto new_snake_ptr = world.CreateSnake();
  world.AddSnake(new_snake_ptr);

  sessions[hdl] = Session(new_snake_ptr->id, GetCurrentTime());
  connections[new_snake_ptr->id] = hdl;

  endpoint.send_binary(hdl, init);

  // send snake
  const auto ses_i = sessions.find(hdl);
  broadcast_binary(packet_add_snake(new_snake_ptr.get()));
  broadcast_binary(packet_move(new_snake_ptr.get()));
  SendPOVUpdateTo(ses_i, new_snake_ptr.get());

  // introduce other snakes in sectors view
  for (auto ptr : world.GetSnakes()) {
    if (ptr.first != new_snake_ptr->id) {
      const Snake *s = ptr.second.get();
      send_binary(ses_i, packet_add_snake(s));
      send_binary(ses_i, packet_move(s));
    }
  }
}

void GameServer::on_message(connection_hdl hdl, message_ptr ptr) {
  if (ptr->get_opcode() != opcode::binary) {
    endpoint.get_alog().write(alevel::app,
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
    endpoint.get_alog().write(alevel::app,
        "Packet '" + std::to_string(packet_type) + "' too big " + std::to_string(len));
    return;
  }

  // session obtain
  const auto ses_i = sessions.find(hdl);
  if (ses_i == sessions.end()) {
    endpoint.get_alog().write(alevel::app, "No session, skip packet");
    return;
  }

  // last client time manage
  Session &ss = ses_i->second;

  // parsing
  if (packet_type <= 250 && len == 1) {
    // in_packet_t_angle, [0 - 250]
    const float angle = Math::f_pi * packet_type / 125.0f;
    DoSnake(ss.snake_id, [=](Snake *s) {
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

      DoSnake(ss.snake_id, [&ss](Snake *s) {
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
      endpoint.get_alog().write(alevel::app,
          "rotate ccw, snake " + std::to_string(ss.snake_id) + ", vfrb " +
          std::to_string(packet_type));
      break;

    case in_packet_t_rot_right:
      buf >> packet_type;  // vfrb (virtual frames count) [0 - 127] of turning
                           // into the right direction
      // snake.eang += mamu * v * snake.scang * snake.spang)
      endpoint.get_alog().write(alevel::app,
          "rotate cw, snake " + std::to_string(ss.snake_id) + ", vfrb " +
          std::to_string(packet_type));
      break;

    case in_packet_t_start_acc:
      DoSnake(ss.snake_id, [](Snake *s) { s->acceleration = true; });
      break;

    case in_packet_t_stop_acc:
      DoSnake(ss.snake_id, [](Snake *s) { s->acceleration = false; });
      break;

    default:
      endpoint.get_alog().write(alevel::app,
          "Unknown packet type " + std::to_string(packet_type) + ", len " +
          std::to_string(ptr->get_payload().size()));
      break;
  }
}

void GameServer::on_close(connection_hdl hdl) {
  const auto ptr = sessions.find(hdl);
  if (ptr != sessions.end()) {
    const snake_id_t snakeId = ptr->second.snake_id;
    sessions.erase(ptr->first);
    RemoveSnake(snakeId);
  }
}

void GameServer::RemoveSnake(snake_id_t id) {
  connections.erase(id);
  world.RemoveSnake(id);
}

PacketInit GameServer::BuildInitPacket() {
  PacketInit init_packet;

  init_packet.game_radius = WorldConfig::game_radius;
  init_packet.max_snake_parts = WorldConfig::max_snake_parts;
  init_packet.sector_size = WorldConfig::sector_size;
  init_packet.sector_count_along_edge = WorldConfig::sector_count_along_edge;

  init_packet.spangdv = Snake::spangdv;
  init_packet.nsp1 = Snake::nsp1;
  init_packet.nsp2 = Snake::nsp2;
  init_packet.nsp3 = Snake::nsp3;

  init_packet.snake_ang_speed = 8.0f * Snake::snake_angular_speed / 1000.0f;
  init_packet.prey_ang_speed = 8.0f * Snake::prey_angular_speed / 1000.0f;
  init_packet.snake_tail_k = Snake::snake_tail_k;

  init_packet.protocol_version = WorldConfig::protocol_version;

  return init_packet;
}

long GameServer::GetCurrentTime() {
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;
  using std::chrono::steady_clock;

  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void GameServer::DoSnake(snake_id_t id, std::function<void(Snake *)> f) {
  if (id > 0) {
    const auto snake_i = world.GetSnake(id);
    if (snake_i->first == id) {
      f(snake_i->second.get());
    }
  }
}

GameServer::SessionMap::iterator GameServer::LoadSessionIter(
    snake_id_t id) {
  const auto hdl_i = connections.find(id);
  if (hdl_i == connections.end()) {
    endpoint.get_alog().write(alevel::app,
        "Failed to locate snake connection " + std::to_string(id));
    return sessions.end();
  }

  const auto ses_i = sessions.find(hdl_i->second);
  if (ses_i == sessions.end()) {
    endpoint.get_alog().write(alevel::app,
        "Failed to locate snake session " + std::to_string(id));
  }

  return ses_i;
}
