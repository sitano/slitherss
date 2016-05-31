#ifndef SLITHER_SERVER_GAME_HPP
#define SLITHER_SERVER_GAME_HPP

#include "server.hpp"
#include "packet/all.hpp"

#include <set>

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
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server m_endpoint;
    con_list m_connections;
};

#endif //SLITHER_SERVER_GAME_HPP
