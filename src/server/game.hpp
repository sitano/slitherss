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

    void on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);

private:
    typedef std::chrono::steady_clock::time_point time_point;
    typedef std::chrono::milliseconds milliseconds;

    time_point get_now_tp();
    void next_tick(time_point last);
    void on_timer(websocketpp::lib::error_code const & ec);

private:
    typedef std::map<connection_hdl, snake::snake_id_t, std::owner_less<connection_hdl>> sessions;

    server m_endpoint;
    server::timer_ptr m_timer;
    time_point m_last_time_point;

    milliseconds timer_interval_ms { 10 };

    sessions m_players;
    world m_world;
    packet_init m_init;
};

#endif //SLITHER_SERVER_GAME_HPP
