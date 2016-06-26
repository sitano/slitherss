#include "game.hpp"

slither_server::slither_server() {
    // set up access channels to only log interesting things
    m_endpoint.clear_access_channels(alevel::all);
    m_endpoint.set_access_channels(alevel::access_core);
    m_endpoint.set_access_channels(alevel::app);

    // Initialize the Asio transport policy
    m_endpoint.init_asio();
    m_endpoint.set_reuse_addr(true);

    // Bind the handlers we are using
    m_endpoint.set_socket_init_handler(bind(&slither_server::on_socket_init, this, ::_1, ::_2));

    m_endpoint.set_open_handler(bind(&slither_server::on_open, this, _1));
    m_endpoint.set_message_handler(bind(&slither_server::on_message, this, _1, _2));
    m_endpoint.set_close_handler(bind(&slither_server::on_close, this, _1));
}

void slither_server::run(game_config config) {
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Running slither server on port " + std::to_string(config.port));

    m_config = config;
    print_world_info();

    m_endpoint.listen(config.port);
    m_endpoint.start_accept();

    m_world.init(config.world);
    m_init = build_init_packet();
    next_tick(get_now_tp());

    try {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Server started...");
        m_endpoint.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

void slither_server::print_world_info() {
    std::stringstream s;
    s << "World info = \n" << m_world;
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, s.str());
}

void slither_server::next_tick(long last) {
    m_last_time_point = last;
    m_timer = m_endpoint.set_timer(
        std::max(0L, timer_interval_ms - (get_now_tp() - last)),
        bind(&slither_server::on_timer, this, _1));
}

void slither_server::on_timer(error_code const & ec) {
    const long now = get_now_tp();
    const long dt = now - m_last_time_point;

    if (ec) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
                "Main game loop timer error: " + ec.message());
        return;
    }

    m_world.tick(dt);
    broadcast_debug();
    broadcast_updates();
    cleanup_dead();

    const long step_time = get_now_tp() - now;
    if (step_time > 10) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
                "Load is too high, step took " + std::to_string(step_time) + "ms");
    }

    next_tick(now);
}

void slither_server::broadcast_debug() {
    if (!m_config.debug) {
        return;
    }

    packet_debug_draw draw;

    for (snake *s: m_world.get_changes()) {
        uint16_t sis = static_cast<uint16_t>(s->id * 1000);

        // bound box
        draw.circles.push_back(d_draw_circle { sis ++, { s->sbb.x, s->sbb.y }, s->sbb.r, 0xc8c8c8 });

        // body inner circles
        const float r1 = 14.0f; // moving snake body radius

        draw.circles.push_back(d_draw_circle { sis ++, { s->get_head_x(), s->get_head_y() }, r1, 0xc80000 });

        const body &sec = *(s->parts.begin() + 1);
        draw.circles.push_back(d_draw_circle { sis ++, { sec.x, sec.y }, r1, 0x3c3c3c });
        draw.circles.push_back(d_draw_circle { sis ++, { sec.x + (s->get_head_x() - sec.x) / 2.0f, sec.y + (s->get_head_y() - sec.y) / 2.0f }, r1, 0x646464 });
        draw.circles.push_back(d_draw_circle { sis ++, { s->parts.back().x, s->parts.back().y }, r1, 0x646464 });

        // bounds
        for (const sector *ss: s->sbb.m_sectors) {
            draw.circles.push_back(d_draw_circle { sis ++, { ss->box.x, ss->box.y }, ss->box.r, 0x511883 });
        }

        // intersection algorithm
        static const size_t head_size = 8;
        static const size_t tail_step = static_cast<size_t>(world_config::sector_size / snake::tail_step_distance);
        static const size_t tail_step_half = tail_step / 2;
        const size_t len = s->parts.size();

        if (len <= head_size + tail_step) {
            for (const body &b : s->parts) {
                draw.circles.push_back(d_draw_circle { sis ++, { b.x, b.y }, world_config::move_step_distance, 0x646464 });
            }
        } else {
            auto p = s->parts[3];
            draw.circles.push_back(d_draw_circle { sis ++, { p.x, p.y }, world_config::sector_size / 2, 0x848484 });
            p = s->parts[0];
            draw.circles.push_back(d_draw_circle { sis ++, { p.x, p.y }, world_config::move_step_distance, 0x646464 });
            p = s->parts[8];
            draw.circles.push_back(d_draw_circle { sis ++, { p.x, p.y }, world_config::move_step_distance, 0x646464 });

            auto end = s->parts.end();
            for (auto i = s->parts.begin() + 7 + tail_step_half; i < end; i += tail_step) {
                draw.circles.push_back(d_draw_circle { sis ++, { i->x, i->y }, world_config::sector_size / 2, 0x848484 });
            }
        }
    }

    if (!draw.empty()) {
        broadcast_debug(draw);
    }
}

void slither_server::broadcast_updates() {
    for (auto ptr: m_world.get_changes()) {
        const snake_id_t id = ptr->id;
        const uint8_t flags = ptr->update;

        if (flags & change_dead) {
            continue;
        }

        if (flags & change_dying) {
            m_endpoint.get_alog().write(websocketpp::log::alevel::app,
                "Found dying snake " + std::to_string(id));

            if (!ptr->bot) {
                const auto ses_i = load_session_i(id);
                if (ses_i != m_sessions.end()) {
                    send_binary(ses_i, packet_end(packet_end::status_death));
                }
            }

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

                if (ptr->fullness >= 100) {
                    ptr->fullness -= 100;
                    ptr->increase_snake();
                    broadcast_binary(packet_inc(ptr));
                } else {
                    broadcast_binary(packet_move(ptr));
                }

                if (!ptr->bot) {
                    const auto ses_i = load_session_i(id);
                    send_pov_update_to(ses_i, ptr);
                    send_food_update_to(ses_i, ptr);
                } else {
                    send_food_update_to(m_sessions.end(), ptr);
                }
            }
        }
    }

    m_world.flush_changes();
}

void slither_server::send_pov_update_to(sessions::iterator ses_i, snake *ptr) {
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

void slither_server::send_food_update_to(sessions::iterator ses_i, snake *ptr) {
    if (!ptr->eaten.empty()) {
        const snake_id_t id = ptr->id;
        for (const food &f : ptr->eaten) {
            // todo: to those who observers me
            broadcast_binary(packet_eat_food(id, f));
            if (ses_i != m_sessions.end()) {
                send_binary(ses_i, packet_fullness(ptr));
            }
        }
        ptr->eaten.clear();
    }
}

void slither_server::cleanup_dead() {
    for (auto id: m_world.get_dead()) {
        remove_snake(id);
    }

    m_world.get_dead().clear();
}

void slither_server::on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s) {
    boost::asio::ip::tcp::no_delay option(true);
    s.set_option(option);
}

void slither_server::on_open(connection_hdl hdl) {
    const auto new_snake_ptr = m_world.create_snake();
    m_world.add_snake(new_snake_ptr);

    m_sessions[hdl] = session(new_snake_ptr->id, get_now_tp());
    m_connections[new_snake_ptr->id] = hdl;

    m_endpoint.send_binary(hdl, m_init);

    // send snake
    const auto ses_i = m_sessions.find(hdl);
    broadcast_binary(packet_add_snake(new_snake_ptr.get()));
    broadcast_binary(packet_move(new_snake_ptr.get()));
    send_pov_update_to(ses_i, new_snake_ptr.get());

    // introduce other snakes in sectors view
    for (auto ptr : m_world.get_snakes()) {
        if (ptr.first != new_snake_ptr->id) {
            const snake *s = ptr.second.get();
            send_binary(ses_i, packet_add_snake(s));
            send_binary(ses_i, packet_move(s));
        }
    }
}

void slither_server::on_message(connection_hdl hdl, message_ptr ptr) {
    if (ptr->get_opcode() != opcode::binary) {
        m_endpoint.get_alog().write(alevel::app,
            "Unknown incoming message opcode " +
                    std::to_string(ptr->get_opcode()));
        return;
    }

    // reader
    std::stringstream buf(ptr->get_payload(), std::ios_base::in);

    in_packet_t packet_type = in_packet_t_angle;
    buf >> packet_type;

    // len check
    const size_t len = ptr->get_payload().size();
    if (len > 255) {
         m_endpoint.get_alog().write(alevel::app,
            "Packet '" + std::to_string(packet_type) + "' too big " + std::to_string(len));
        return;
    }

    // session obtain
    const auto ses_i = m_sessions.find(hdl);
    if (ses_i == m_sessions.end()) {
        m_endpoint.get_alog().write(alevel::app, "No session, skip packet");
        return;
    }

    // last client time manage
    session &ss = ses_i->second;

    // parsing
    if (packet_type <= 250 && len == 1) {
        // in_packet_t_angle, [0 - 250]
        const float angle = world::f_pi * packet_type / 125.0f;
        do_snake(ss.snake_id, [=](snake *s){
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

            do_snake(ss.snake_id, [&ss](snake *s){
                s->name = ss.name;
                s->skin = ss.skin;
            });
            break;

        case in_packet_t_victory_message:
            buf >> packet_type; // always 118
            buf.str(ss.message);
            break;

        case in_packet_t_rot_left:
            buf >> packet_type; // vfrb (virtual frames count) [0 - 127] of turning into the right direction
            // snake.eang -= mamu * v * snake.scang * snake.spang)
             m_endpoint.get_alog().write(alevel::app,
                "rotate ccw, snake " + std::to_string(ss.snake_id) + ", vfrb " + std::to_string(packet_type));
            break;

        case in_packet_t_rot_right:
            buf >> packet_type; // vfrb (virtual frames count) [0 - 127] of turning into the right direction
            // snake.eang += mamu * v * snake.scang * snake.spang)
            m_endpoint.get_alog().write(alevel::app,
                "rotate cw, snake " + std::to_string(ss.snake_id) + ", vfrb " + std::to_string(packet_type));
            break;

        case in_packet_t_start_acc:
            do_snake(ss.snake_id, [](snake *s){ s->acceleration = true; });
            break;

        case in_packet_t_stop_acc:
            do_snake(ss.snake_id, [](snake *s){ s->acceleration = false; });
            break;

        default:
            m_endpoint.get_alog().write(alevel::app,
                "Unknown packet type " + std::to_string(packet_type) +
                ", len " + std::to_string(ptr->get_payload().size()));
            break;
    }
}

void slither_server::on_close(connection_hdl hdl) {
    const auto ptr = m_sessions.find(hdl);
    if (ptr != m_sessions.end()) {
        const snake_id_t snakeId = ptr->second.snake_id;
        m_sessions.erase(ptr->first);
        remove_snake(snakeId);
    }
}

void slither_server::remove_snake(snake_id_t id) {
        m_connections.erase(id);
        m_world.remove_snake(id);
}

packet_init slither_server::build_init_packet() {
    packet_init init;

    init.game_radius = world_config::game_radius;
    init.max_snake_parts = world_config::max_snake_parts;
    init.sector_size = world_config::sector_size;
    init.sector_count_along_edge = world_config::sector_count_along_edge;

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

long slither_server::get_now_tp() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void slither_server::do_snake(snake_id_t id, std::function<void(snake *)> f) {
    if (id > 0) {
        const auto snake_i = m_world.get_snake(id);
        if (snake_i->first == id) {
            f(snake_i->second.get());
        }
    }
}

slither_server::sessions::iterator slither_server::load_session_i(snake_id_t id) {
    const auto hdl_i = m_connections.find(id);
    if (hdl_i == m_connections.end()) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
            "Failed to locate snake connection " + std::to_string(id));
        return m_sessions.end();
    }

    const auto ses_i = m_sessions.find(hdl_i->second);
    if (ses_i == m_sessions.end()) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
            "Failed to locate snake session " + std::to_string(id));
    }

    return ses_i;
}




