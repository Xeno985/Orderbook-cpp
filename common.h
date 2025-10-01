// common.h
#ifndef COMMON_H
#define COMMON_H
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>


typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
//Forward declare classes 
class OrderManager;
class Authenticator;
class Order;
struct Trade;

#endif