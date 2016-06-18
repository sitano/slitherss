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

    packet_debug_reset reset;
    packet_debug_draw draw;

    for (auto ptr: m_world.get_snakes()) {
        const snake *s = ptr.second.get();
        for (const body &b : s->parts) {
            // body
            draw.circles.push_back(d_draw_circle {
                    static_cast<uint16_t>(b.x),
                    static_cast<uint16_t>(b.y),
                    snake::move_step_distance * snake::move_step_distance,
                    102 });
        }

        // body
        draw.circles.push_back(d_draw_circle {
                static_cast<uint16_t>(s->box.x),
                static_cast<uint16_t>(s->box.y),
                static_cast<uint16_t>(s->box.r2),
                255 });
    }

    broadcast_binary(reset);
    broadcast_binary(draw);
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

                broadcast_binary(packet_move_rel { id,
                        static_cast<int8_t>(ptr->get_head_dx()),
                        static_cast<int8_t>(ptr->get_head_dy()) });

                if (!ptr->bot) {
                    const auto ses_i = load_session_i(id);
                    if (ses_i != m_sessions.end()) {
                        send_pov_update_to(ses_i, ptr);
                    }
                }
            }
        }
    }

    m_world.flush_changes();
}

void slither_server::send_pov_update_to(sessions::iterator ses_i, snake *ptr) {
    if (!ptr->box.new_sectors.empty()) {
        for (const auto s_ptr : ptr->box.new_sectors) {
            send_binary(ses_i, packet_add_sector(s_ptr->x, s_ptr->y));
            send_binary(ses_i, packet_set_food(&s_ptr->m_food));
        }
        ptr->box.new_sectors.clear();
    }

    if (!ptr->box.old_sectors.empty()) {
        for (const auto s_ptr : ptr->box.old_sectors) {
            send_binary(ses_i, packet_remove_sector(s_ptr->x, s_ptr->y));
        }
        ptr->box.old_sectors.clear();
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
    const auto ptr = m_world.create_snake();
    m_world.add_snake(ptr);

    m_sessions[hdl] = session(ptr->id, get_now_tp());
    m_connections[ptr->id] = hdl;

    m_endpoint.send_binary(hdl, m_init);
    // TODO: send sectors packets
    // TODO: send food packets
    // send snake
    const auto ses_i = m_sessions.find(hdl);
    broadcast_binary(packet_add_snake(ptr));
    broadcast_binary(packet_move {
        ptr->id, static_cast<uint16_t>(ptr->get_head_x()), static_cast<uint16_t>(ptr->get_head_y()) });
    send_pov_update_to(ses_i, ptr.get());

    // todo introduce other snakes in sectors view
    for (auto snake : m_world.get_snakes()) {
        if (snake.first != ptr->id) {
            const std::shared_ptr<::snake> &ptr2 = snake.second;
            send_binary(ses_i, packet_add_snake(snake.second));
            send_binary(ses_i, packet_move {
                ptr2->id, static_cast<uint16_t>(ptr2->get_head_x()), static_cast<uint16_t>(ptr2->get_head_y()) });
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




