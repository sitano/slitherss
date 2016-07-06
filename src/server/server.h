#ifndef SLITHER_SERVER_SERVER_HPP
#define SLITHER_SERVER_SERVER_HPP

#include "config.h"
#include "streambuf_array.h"

#include <websocketpp/server.hpp>

typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::frame::opcode::value opcode;
typedef websocketpp::lib::error_code error_code;

class server : public websocketpp::server<slither_server_config> {
 public:
  template <typename T>
  void send(connection_hdl hdl, T packet, opcode op, error_code &ec) {
    const connection_ptr con = get_con_from_hdl(hdl, ec);
    if (ec) {
      return;
    }

    const size_t max = packet.get_size();
    if (max <= 128) {
      streambuf_array<128> buf;
      std::ostream out(&buf);
      out << packet;
      ec = con->send(boost::asio::buffer_cast<void const *>(buf.data()),
                     buf.size(), op);
    } else {
      boost::asio::streambuf buf(max);
      buf.prepare(max);

      std::ostream out(&buf);
      out << packet;

      ec = con->send(boost::asio::buffer_cast<void const *>(buf.data()),
                     buf.size(), op);
    }
  }

  template <typename T>
  void send_binary(connection_hdl hdl, T packet, error_code &ec) {
    send(hdl, packet, opcode::binary, ec);
  }

  template <typename T>
  void send_binary(connection_hdl hdl, T packet) {
    error_code ec;
    send_binary(hdl, packet, ec);
    if (ec) {
      get_alog().write(alevel::app, "Write Error: " + ec.message());
    }
  }
};

typedef server::message_ptr message_ptr;

#endif  // SLITHER_SERVER_SERVER_HPP
