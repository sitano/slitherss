#ifndef SRC_SERVER_GAME_H_
#define SRC_SERVER_GAME_H_

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "server/server.h"

#include "game/world.h"

#include "packet/d_all.h"
#include "packet/p_all.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

struct Session {
  snake_id_t snake_id = 0;
  long last_packet_time = 0;

  std::string name;
  std::string message;

  uint8_t protocol_version = 0;  // current 8
  uint8_t skin = 0;              // 0 - 39

  Session() = default;
  Session(snake_id_t id, long now) : snake_id(id), last_packet_time(now) {}
};

class GameServer {
 public:
  GameServer();

  int Run(IncomingConfig in_config);

  PacketInit BuildInitPacket();

  typedef std::unordered_map<snake_id_t, connection_hdl> ConnectionMap;
  typedef std::map<connection_hdl, Session, std::owner_less<connection_hdl>> SessionMap;
  typedef SessionMap::iterator SessionIter;

 private:
  void on_socket_init(connection_hdl, boost::asio::ip::tcp::socket &s);  // NOLINT(runtime/references)
  void on_open(connection_hdl hdl);
  void on_message(connection_hdl hdl, message_ptr ptr);
  void on_close(connection_hdl hdl);
  void on_timer(error_code const &ec);

  void SendPOVUpdateTo(SessionIter ses_i, Snake *ptr);
  void SendFoodUpdate(Snake *ptr);
  void BroadcastDebug();
  void BroadcastUpdates();
  SessionIter LoadSessionIter(snake_id_t id);

  void DoSnake(snake_id_t id, std::function<void(Snake *)> f);
  void RemoveSnake(snake_id_t id);
  void RemoveDeadSnakes();

  long GetCurrentTime();
  void NextTick(long last);

  void PrintWorldInfo();

 private:
  template <typename T>
  void send_binary(SessionMap::iterator s, T packet) {
    const long now = GetCurrentTime();
    const uint16_t interval =
        static_cast<uint16_t>(now - s->second.last_packet_time);
    s->second.last_packet_time = now;
    packet.client_time = interval;
    endpoint.send_binary(s->first, packet);
  }

  template <typename T>
  void broadcast_binary(T packet) {
    const long now = GetCurrentTime();
    for (auto &s : sessions) {
      const uint16_t interval =
          static_cast<uint16_t>(now - s.second.last_packet_time);
      s.second.last_packet_time = now;
      packet.client_time = interval;
      endpoint.send_binary(s.first, packet);
    }
  }

  template <typename T>
  void broadcast_debug(T packet) {
    for (auto &s : sessions) {
      endpoint.send_binary(s.first, packet);
    }
  }

  WSPPServer endpoint;

  WSPPServer::timer_ptr timer;
  long last_time_point;
  static const long timer_interval_ms = 10;

  World world;
  PacketInit init;
  IncomingConfig config;

  // TODO(john.koepi): reserve to collections
  SessionMap sessions;
  ConnectionMap connections;
};

#endif  // SRC_SERVER_GAME_H_
