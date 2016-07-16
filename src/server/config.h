#ifndef SRC_SERVER_CONFIG_H_
#define SRC_SERVER_CONFIG_H_

#include "game/config.h"

#include <websocketpp/config/asio_no_tls.hpp>
// #include <websocketpp/extensions/permessage_deflate/enabled.hpp>

using websocketpp::log::alevel;
using websocketpp::log::elevel;

struct IncomingConfig {
  uint16_t port = 8080;

  bool help = false;
  bool version = false;
  bool verbose = false;
  bool debug = false;

  WorldConfig world;
};

IncomingConfig ParseCommandLine(const int argc, const char *const *argv);

struct WSPPServerConfig : public websocketpp::config::asio {
  // pull default settings from our core config
  typedef websocketpp::config::asio core;

  typedef core::concurrency_type concurrency_type;
  typedef core::request_type request_type;
  typedef core::response_type response_type;
  typedef core::message_type message_type;

  // TODO(john.koepi): provide pool managers
  typedef core::con_msg_manager_type con_msg_manager_type;
  typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

  typedef core::alog_type alog_type;
  typedef core::elog_type elog_type;
  typedef core::rng_type rng_type;
  typedef core::endpoint_base endpoint_base;

  static bool const enable_multithreading = true;

  struct transport_config : public core::transport_config {
    typedef core::concurrency_type concurrency_type;
    typedef core::elog_type elog_type;
    typedef core::alog_type alog_type;
    typedef core::request_type request_type;
    typedef core::response_type response_type;

    static bool const enable_multithreading = true;
  };

  typedef websocketpp::transport::asio::endpoint<transport_config> transport_type;

  /// static const websocketpp::log::level elog_level =
  ///    websocketpp::log::elevel::none;
  /// static const websocketpp::log::level alog_level =
  ///    websocketpp::log::alevel::none;

  /// permessage_compress extension
  // struct permessage_deflate_config {};

  // typedef websocketpp::extensions::permessage_deflate::enabled
  //    <permessage_deflate_config> permessage_deflate_type;
};

#endif  // SRC_SERVER_CONFIG_H_
