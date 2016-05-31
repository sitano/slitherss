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

private:
    typedef std::map<connection_hdl, std::unique_ptr<snake>, std::owner_less<connection_hdl>> players;

    server m_endpoint;
    players m_players;
};

#endif //SLITHER_SERVER_GAME_HPP
