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

struct session {
    snake::snake_id_t snake_id;
    long last_packet_time = 0;

    std::string name;
    std::string message;

    uint8_t protocol_version = 0; // current 8
    uint8_t skin = 0; // 0 - 39

    session() = default;
    explicit session(snake::snake_id_t id) : snake_id(id) {}
};

class slither_server {
public:
    slither_server();

    void run(uint16_t port);

    packet_init build_init_packet();

private:
    void on_socket_init(connection_hdl, boost::asio::ip::tcp::socket & s);
    void on_open(connection_hdl hdl);
    void on_message(connection_hdl hdl, message_ptr ptr);
    void on_close(connection_hdl hdl);

    void on_timer(error_code const & ec);
    void broadcast_updates();

    long get_now_tp();
    void next_tick(long last);

    void do_snake(snake::snake_id_t id, std::function<void(snake*)> f);

private:
    typedef std::map<connection_hdl, session, std::owner_less<connection_hdl>> sessions;
    typedef std::unordered_map<snake::snake_id_t, connection_hdl> connections;

    server m_endpoint;

    server::timer_ptr m_timer;
    long m_last_time_point;
    static const long timer_interval_ms = 10;

    world m_world;
    packet_init m_init;

    // todo: reserve to collections
    sessions m_sessions;
    connections m_connections;
};

#endif //SLITHER_SERVER_GAME_HPP
