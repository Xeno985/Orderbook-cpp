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

// Menu interface
void displayMenu() {
    std::cout << "1. Place Order\n";
    std::cout << "2. Cancel Order\n";
    std::cout << "3. Modify Order\n";
    std::cout << "4. View Current Positions\n";
    std::cout << "5. Get Order History by Currency\n";
    std::cout << "6. Get Order History by Instrument\n";
    std::cout << "7. Stream Market Data\n";
    std::cout << "8. Get Summary by Instrument\n";
    std::cout << "9. Get Summary by Currency\n";
    std::cout << "10. Get Ticker Data\n";
    std::cout << "11. Get Contract Size\n";
    std::cout << "12. Get All Supported Currencies\n";
    std::cout << "13. Subscribe to Channel\n";
    std::cout << "14. Unsubscribe from Channel\n";
    std::cout << "15. Unsubscribe All\n";
    std::cout << "16. Exit\n";
    std::cout << "Enter your choice: ";
}

void placeOrderMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string orderId, instrumentName, side, orderTypeStr, label;
    double amount, price;
    InstrumentType instrumentType;

    std::cout << "Enter Order ID: ";
    std::cin >> orderId;
    std::cout << "Enter Instrument Name: ";
    std::cin >> instrumentName;
    std::cout << "Enter Instrument Type (Spot/Futures/Options): ";
    std::string instrTypeStr;
    std::cin >> instrTypeStr;
    instrumentType = parseInstrumentType(instrTypeStr);
    std::cout << "Enter Side (buy/sell): ";
    std::cin >> side;
    std::cout << "Enter Order Type (market/limit/stop_limit): ";
    std::cin >> orderTypeStr;
    std::cout << "Enter Amount: ";
    std::cin >> amount;
    std::cout << "Enter Price (0 for market orders): ";
    std::cin >> price;
    std::cout << "Enter Label: ";
    std::cin >> label;

    Order order(orderId, instrumentName, instrumentType, side, orderTypeStr, amount, price, std::nullopt, std::nullopt, label);
    manager->placeOrder(order);
}

void cancelOrderMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string orderId;
    std::cout << "Enter Order ID to cancel: ";
    std::cin >> orderId;
    manager->cancelOrder(orderId);
}

void modifyOrderMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }
    bool post_only,reduce_only;
    std::string orderId, advanced;
    double newPrice, newAmount;
    std::cout << "Enter Order ID to modify: ";
    std::cin >> orderId;
    std::cout << "Enter New Amount: ";
    std::cin >> newAmount;
    std::cout << "Enter New Price: ";
    std::cin >> newPrice;
    std::cout << "Enter Advanced Option(uds/implv): ";
    std::cin >> advanced;
    std::cout << "Do you want it to be post only?(true/false) ";
    std::cin>>post_only;
    std::cout << "Do you want it to be reduce only?(true/false) ";
    std::cin>>reduce_only;

    manager->modifyOrder(orderId, newPrice, newAmount, advanced,post_only,reduce_only);
}

void viewPositionsMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    auto positions = manager->getCurrentPositions();
    std::cout << "Current Positions:\n";
    for (const auto& pair : positions) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }
}

void getOrderHistoryByCurrencyMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string currency;
    std::cout << "Enter Currency (e.g., BTC, ETH): ";
    std::cin >> currency;
    manager->getOrderHistoryByCurrency(currency);
}

void getOrderHistoryByInstrumentMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getOrderHistoryByInstrument(instrument);
}

void streamMarketDataMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string channel;
    std::cout << "Enter Market Data Channel (e.g., ticker.BTC-PERPETUAL.raw): ";
    std::cin >> channel;
    manager->streamMarketData(channel);
}

void getSummaryByInstrumentMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getSummaryByInstrument(instrument);
}

void getSummaryByCurrencyMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string currency;
    std::cout << "Enter Currency (e.g., BTC, ETH): ";
    std::cin >> currency;
    manager->getSummaryByCurrency(currency);
}

void getTickerDataMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getTickerData(instrument);
}

void getContractSizeMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getContractSize(instrument);
}

void getAllSupportedCurrenciesMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    manager->getAllSupportedCurrencies();
}

void subscribeMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string channel;
    std::cout << "Enter Channel to Subscribe (e.g., ticker.BTC-PERPETUAL.raw): ";
    std::cin >> channel;
    manager->subscribe(channel);
}

void unsubscribeMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string channel;
    std::cout << "Enter Channel to Unsubscribe (e.g., ticker.BTC-PERPETUAL.raw): ";
    std::cin >> channel;
    manager->unsubscribe(channel);
}

void unsubscribeAllMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    manager->unsubscribeAll();
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