#pragma once
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <vector>
#include "common.h"

class Authenticator {
private:
    const std::string client_id = get_client_id();
    const std::string client_secret = get_client_secret();

    std::string auth_message;

    std::string get_client_id() {
        const char* client_id = std::getenv("CLIENT_ID");
        if (client_id != nullptr) {
            return std::string(client_id);
        } else {
            std::cerr << "Error: CLIENT_ID environment variable is not set." << std::endl;
            return "";
        }
    }

    std::string get_client_secret() {
        const char* client_secret = std::getenv("CLIENT_SECRET");
        if (client_secret != nullptr) {
            return std::string(client_secret);
        } else {
            std::cerr << "Error: CLIENT_SECRET environment variable is not set." << std::endl;
            return "";
        }
    }

public:
  Authenticator();

  void send_authcall(client* c,websocketpp::connection_hdl hdl);
};
