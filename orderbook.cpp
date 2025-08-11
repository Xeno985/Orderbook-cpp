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
#include "Order.h"
#include "Authenticator.h"
#include "types.hpp"
#include "OrderManager.h"


// Typedefs and using declarations
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// Global variables
std::mutex mtx;
std::condition_variable cv;
bool connectionOpen = false;



OrderManager* manager = nullptr;

// WebSocket event handlers
void on_open(websocketpp::connection_hdl hdl, client* c) {
    std::cout << "WebSocket connection opened!" << std::endl;
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c->get_con_from_hdl(hdl, ec);

    if (ec) {
        std::cout << "Failed to get connection pointer: " << ec.message() << std::endl;
        return;
    }
    Authenticator auth;
    manager = new OrderManager(c, hdl);
    // Authenticate
    auth.send_authcall(c, hdl);

    // Notify the menu thread that the connection is open
    {
        std::lock_guard<std::mutex> lock(mtx);
        connectionOpen = true;
    }
    cv.notify_one();
}

void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    std::string payload = msg->get_payload();
    try {
        auto jsonData = nlohmann::json::parse(payload);

        if (jsonData.contains("result")) {
            if (manager) {
                manager->processApiResponse(payload);
            } else {
                std::cerr << "Manager is not initialized!" << std::endl;
            }
        }

        // For other messages, process as needed
        std::cout << "Message received: " << jsonData.dump(4) << "\n";
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

void on_fail(websocketpp::connection_hdl hdl) {
    std::cout << "WebSocket connection failed!" << std::endl;
}

void on_close(websocketpp::connection_hdl hdl) {
    std::cout << "WebSocket connection closed!" << std::endl;

    if (manager) {
        delete manager;
        manager = nullptr;
    }

    // Notify the menu thread that the connection is closed
    {
        std::lock_guard<std::mutex> lock(mtx);
        connectionOpen = false;
    }
}

context_ptr on_tls_init(const char* hostname, websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cout << "TLS Initialization Error: " << e.what() << std::endl;
    }

    return ctx;
}


// Main function
int main(int argc, char* argv[]) {
    client c;
    std::string hostname = "test.deribit.com/ws/api/v2";
    std::string uri = "wss://" + hostname;

    try {
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);
        c.init_asio();

        c.set_message_handler(&on_message);
        c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));
        c.set_open_handler(bind(&on_open, ::_1, &c));
        c.set_fail_handler(bind(&on_fail, ::_1));
        c.set_close_handler([](websocketpp::connection_hdl hdl) {
            on_close(hdl);
        });

        c.set_error_channels(websocketpp::log::elevel::all);  // Enable detailed error logging
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "Could not create connection because: " << ec.message() << std::endl;
            return 0;
        }
        c.connect(con);

        // Menu loop in a separate thread
        std::thread menuThread([&] {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return connectionOpen; });
            lock.unlock();

            while (true) {
                displayMenu();
                int choice;
                std::cin >> choice;
                switch (choice) {
                    case 1:
                        placeOrderMenu(manager);
                        break;
                    case 2:
                        cancelOrderMenu(manager);
                        break;
                    case 3:
                        modifyOrderMenu(manager);
                        break;
                    case 4:
                        viewPositionsMenu(manager);
                        break;
                    case 5:
                        getOrderHistoryByCurrencyMenu(manager);
                        break;
                    case 6:
                        getOrderHistoryByInstrumentMenu(manager);
                        break;
                    case 7:
                        streamMarketDataMenu(manager);
                        break;
                    case 8:
                        getSummaryByInstrumentMenu(manager);
                        break;
                    case 9:
                        getSummaryByCurrencyMenu(manager);
                        break;
                    case 10:
                        getTickerDataMenu(manager);
                        break;
                    case 11:
                        getContractSizeMenu(manager);
                        break;
                    case 12:
                        getAllSupportedCurrenciesMenu(manager);
                        break;
                    case 13:
                        subscribeMenu(manager);
                        break;
                    case 14:
                        unsubscribeMenu(manager);
                        break;
                    case 15:
                        unsubscribeAllMenu(manager);
                        break;
                    case 16:
                        std::cout << "Exiting...\n";
                        return 0;
                    default:
                        std::cout << "Invalid choice. Please try again.\n";
                }

                if (manager == nullptr) {
                    std::cerr << "Connection closed. Exiting menu." << std::endl;
                    break;
                }
            }
            return 0;
        });

        c.run();
        menuThread.join();
    } catch (websocketpp::exception const& e) {
        std::cout << "WebSocket Exception: " << e.what() << std::endl;
    }
}