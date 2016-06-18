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
    snake_id_t snake_id = 0;
    long last_packet_time = 0;

    std::string name;
    std::string message;

    uint8_t protocol_version = 0; // current 8
    uint8_t skin = 0; // 0 - 39

    session() = default;
    session(snake_id_t id, long now) : snake_id(id), last_packet_time(now) {}
};

class slither_server {
public:
    slither_server();

    void run(game_config config);

    packet_init build_init_packet();

    typedef std::map<connection_hdl, session, std::owner_less<connection_hdl>> sessions;
    typedef std::unordered_map<snake_id_t, connection_hdl> connections;

private:
    void on_socket_init(connection_hdl, boost::asio::ip::tcp::socket & s);
    void on_open(connection_hdl hdl);
    void on_message(connection_hdl hdl, message_ptr ptr);
    void on_close(connection_hdl hdl);
    void remove_snake(snake_id_t id);

    void on_timer(error_code const & ec);
    void broadcast_debug();
    void broadcast_updates();
    void send_pov_update_to(sessions::iterator ses_i, snake *ptr);
    void cleanup_dead();

    long get_now_tp();
    void next_tick(long last);
    sessions::iterator load_session_i(snake_id_t id);

    void do_snake(snake_id_t id, std::function<void(snake*)> f);
    void print_world_info();

private:

    template <typename T>
    void send_binary(sessions::iterator s, T packet) {
        const long now = get_now_tp();
        const uint16_t interval = static_cast<uint16_t>(now - s->second.last_packet_time);
        s->second.last_packet_time = now;
        packet.client_time = interval;
        m_endpoint.send_binary(s->first, packet);
    }

    template <typename T>
    void broadcast_binary(T packet) {
        const long now = get_now_tp();
        for (auto &s: m_sessions) {
            const uint16_t interval = static_cast<uint16_t>(now - s.second.last_packet_time);
            s.second.last_packet_time = now;
            packet.client_time = interval;
            m_endpoint.send_binary(s.first, packet);
        }
    }

    server m_endpoint;

    server::timer_ptr m_timer;
    long m_last_time_point;
    static const long timer_interval_ms = 10;

    world m_world;
    packet_init m_init;
    game_config m_config;

    // todo: reserve to collections
    sessions m_sessions;
    connections m_connections;
};

#endif //SLITHER_SERVER_GAME_HPP
