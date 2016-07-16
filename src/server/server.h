#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include <websocketpp/server.hpp>

#include "server/config.h"
#include "server/streambuf_array.h"

typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::frame::opcode::value opcode;
typedef websocketpp::lib::error_code error_code;

class WSPPServer : public websocketpp::server<WSPPServerConfig> {
 public:
  template <typename T>
  void send(connection_hdl hdl, T packet, opcode op, error_code &ec) {  // NOLINT(runtime/references)
    const connection_ptr con = get_con_from_hdl(hdl, ec);
    if (ec) {
      return;
    }

    const size_t max = packet.get_size();
    if (max <= 128) {
      streambuf_array<128> buf;
      std::ostream out(&buf);
      out << packet;
      ec = con->send(boost::asio::buffer_cast<void const *>(buf.data()), buf.size(), op);
    } else {
      boost::asio::streambuf buf(max);
      buf.prepare(max);

      std::ostream out(&buf);
      out << packet;

      ec = con->send(boost::asio::buffer_cast<void const *>(buf.data()), buf.size(), op);
    }
  }

  template <typename T>
  void send_binary(connection_hdl hdl, T packet, error_code &ec) {  // NOLINT(runtime/references)
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

typedef WSPPServer::message_ptr message_ptr;

#endif  // SRC_SERVER_SERVER_H_
