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

    // game init
    init_random();

    try {
        m_endpoint.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

void slither_server::on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s) {
    boost::asio::ip::tcp::no_delay option(true);
    s.set_option(option);
}

void slither_server::on_open(connection_hdl hdl) {
    auto snake = create_snake();
    m_players[hdl] = snake;

    m_endpoint.send_binary(hdl, packet_init());
    // TODO: send sectors packets
    // TODO: send food packets
    // send snake
    m_endpoint.send_binary(hdl, packet_add_snake(snake));
}

void slither_server::on_close(connection_hdl hdl) {
    m_players.erase(hdl);
}

std::shared_ptr<snake> slither_server::create_snake() {
    lastSnakeId ++;

    float angle = M_2PI * next_random(255) / 256.0;

    auto s = new snake();
    s->id = lastSnakeId;
    s->name = "";
    s->color = static_cast<uint8_t>(9 + next_random(21 - 9 + 1));
    s->x = m_init.game_radius + next_random(1000) - 500;
    s->y = m_init.game_radius + next_random(1000) - 500;
    s->speed = 5.79f;
    s->angle = angle;
    s->wangle = angle;
    s->eangle = angle;
    s->ehang = angle;
    s->wehang = angle;
    s->fullness = 0.0f;
    s->parts = {
            {s->x, s->y},
            {s->x, s->y}
    };

    return std::shared_ptr<snake>(s);
}

void slither_server::init_random() {
    std::srand(std::time(nullptr));
}

int slither_server::next_random() {
    return std::rand();
}

template<typename T>
T slither_server::next_random(T base) {
    return static_cast<T>(next_random() % base);
}








