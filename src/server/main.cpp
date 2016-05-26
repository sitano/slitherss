#include "server/config.hpp"
#include "packet/init.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <streambuf>
#include <string>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

void on_socket_init(websocketpp::connection_hdl, boost::asio::ip::tcp::socket & s) {
    boost::asio::ip::tcp::no_delay option(true);
    s.set_option(option);
}

class slither_server {
public:
    typedef websocketpp::connection_hdl connection_hdl;

    slither_server() {
        // set up access channels to only log interesting things
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
        m_endpoint.set_access_channels(websocketpp::log::alevel::app);

        // Initialize the Asio transport policy
        m_endpoint.init_asio();
        m_endpoint.set_reuse_addr(true);

        // Bind the handlers we are using
        m_endpoint.set_open_handler(bind(&slither_server::on_open, this, _1));
        m_endpoint.set_close_handler(bind(&slither_server::on_close, this, _1));
    }

    void run(uint16_t port) {
        std::stringstream ss;
        ss << "Running slither server on port " << port;
        m_endpoint.get_alog().write(websocketpp::log::alevel::app, ss.str());

        m_endpoint.listen(port);
        m_endpoint.start_accept();

        try {
            m_endpoint.run();
        } catch (websocketpp::exception const & e) {
            std::cout << e.what() << std::endl;
        }
    }

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);

        // Test
        boost::asio::streambuf buf(26);
        std::ostream out(&buf);

        packet_init p;
        out << p;

        auto data = boost::asio::buffer_cast<void const *>(buf.data());
        websocketpp::lib::error_code ec;
        m_endpoint.send(hdl, data, buf.size(), websocketpp::frame::opcode::binary, ec);
        if (ec) {
            m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Write Error: " + ec.message());
        } else {
            m_endpoint.get_alog().write(websocketpp::log::alevel::app, std::string("Packet Length: ") + std::to_string(buf.size()));
        }
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
    }
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server m_endpoint;
    con_list m_connections;
};

int main(int argc, char* argv[]) {
    slither_server s;

    uint16_t port = 8080;

    if (argc < 2) {
        std::cout << "Usage: slither_server [port]" << std::endl;
        return 1;
    }

    if (argc >= 2) {
        int i = atoi(argv[1]);
        if (i <= 0 || i > 65535) {
            std::cout << "invalid port" << std::endl;
            return 1;
        }

        port = uint16_t(i);
    }

    s.run(port);
    return 0;
}
