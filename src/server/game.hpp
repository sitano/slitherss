#ifndef SLITHER_SERVER_GAME_HPP
#define SLITHER_SERVER_GAME_HPP

#include "server.hpp"
#include "packet/all.hpp"

#include <map>
#include <memory>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class slither_server {
public:
    slither_server();

    void run(uint16_t port);

    void on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);

    std::shared_ptr<snake> create_snake();
    void init_random();
    int next_random();
    template <typename T> T next_random(T base);
private:
    typedef std::map<connection_hdl, std::shared_ptr<snake>, std::owner_less<connection_hdl>> sessions;

    server m_endpoint;
    sessions m_players;
    packet_init m_init;

    uint16_t lastSnakeId = 0;
};

#endif //SLITHER_SERVER_GAME_HPP
