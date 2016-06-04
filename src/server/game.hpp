#ifndef SLITHER_SERVER_GAME_HPP
#define SLITHER_SERVER_GAME_HPP

#include "server.hpp"
#include "game/world.hpp"
#include "packet/p_all.hpp"

#include <map>
#include <memory>
#include <chrono>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class slither_server {
public:
    slither_server();

    void run(uint16_t port);

    packet_init build_init_packet();

private:
    void on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);

    void on_timer(websocketpp::lib::error_code const & ec);
    void broadcast_updates();

    long get_now_tp();
    void next_tick(long last);

private:
    typedef std::map<connection_hdl, snake::snake_id_t, std::owner_less<connection_hdl>> sessions_snakes;
    typedef std::unordered_map<snake::snake_id_t, connection_hdl> snakes_sessions;

    server m_endpoint;

    server::timer_ptr m_timer;
    long m_last_time_point;
    static const long timer_interval_ms = 10;

    world m_world;
    packet_init m_init;

    sessions_snakes m_sessions;
    snakes_sessions m_snakes;
};

#endif //SLITHER_SERVER_GAME_HPP
