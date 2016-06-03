#include "game.hpp"

slither_server::slither_server() {
    // set up access channels to only log interesting things
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
    m_endpoint.set_access_channels(websocketpp::log::alevel::app);

    // Initialize the Asio transport policy
    m_endpoint.init_asio();
    m_endpoint.set_reuse_addr(true);

    // Bind the handlers we are using
    m_endpoint.set_socket_init_handler(bind(&slither_server::on_socket_init, this, ::_1, ::_2));

    m_endpoint.set_open_handler(bind(&slither_server::on_open, this, _1));
    m_endpoint.set_close_handler(bind(&slither_server::on_close, this, _1));
}

void slither_server::run(uint16_t port) {
    std::stringstream ss;
    ss << "Running slither server on port " << port;

    m_endpoint.get_alog().write(websocketpp::log::alevel::app, ss.str());

    m_endpoint.listen(port);
    m_endpoint.start_accept();

    m_world.init();
    m_init = build_init_packet();
    next_tick(get_now_tp());

    try {
        m_endpoint.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

void slither_server::next_tick(time_point last) {
    m_last_time_point = last;
    auto diff = std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now() - last);
    auto diff2 = timer_interval_ms - diff;
    m_timer = m_endpoint.set_timer(
        std::max(0L, diff2.count()),
        websocketpp::lib::bind(
            &slither_server::on_timer,
            this,
            websocketpp::lib::placeholders::_1
        )
    );
}

void slither_server::on_timer(websocketpp::lib::error_code const & ec) {
    time_point now = get_now_tp();
    milliseconds interval = std::chrono::duration_cast<milliseconds>(now - m_last_time_point);

    if (ec) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,
                "Main game loop timer error: " + ec.message());
        return;
    }

    m_world.tick(interval.count());

    next_tick(now);
}

void slither_server::on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s) {
    boost::asio::ip::tcp::no_delay option(true);
    s.set_option(option);
}

void slither_server::on_open(connection_hdl hdl) {
    auto snake = m_world.create_snake();
    m_world.add_snake(snake);
    m_players[hdl] = snake->id;

    m_endpoint.send_binary(hdl, m_init);
    // TODO: send sectors packets
    // TODO: send food packets
    // send snake
    m_endpoint.send_binary(hdl, packet_add_snake(snake));
}

void slither_server::on_close(connection_hdl hdl) {
    auto id = m_players[hdl];
    m_players.erase(hdl);
    m_world.remove_snake(id);
}

packet_init slither_server::build_init_packet() {
    packet_init init;

    init.game_radius = m_world.game_radius;
    init.max_snake_parts = m_world.max_snake_parts;
    init.sector_size = m_world.sector_size;
    init.sector_count_along_edge = m_world.sector_count_along_edge;

    init.spangdv = snake::spangdv;
    init.nsp1 = snake::nsp1;
    init.nsp2 = snake::nsp2;
    init.nsp3 = snake::nsp3;

    init.snake_ang_speed = snake::snake_ang_speed;
    init.prey_ang_speed = snake::prey_ang_speed;
    init.snake_tail_k = snake::snake_tail_k;

    init.protocol_version = world::protocol_version;

    return init;
}

slither_server::time_point slither_server::get_now_tp() {
    return std::chrono::steady_clock::now();
}


