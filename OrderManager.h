
#include<map>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>


// OrderManager class
class OrderManager {
private:
    std::unordered_map<std::string, Order> orders;
    client* wsClient;
    websocketpp::connection_hdl wsHandle;

    bool orderExists(const std::string& orderId) const {
        return orders.find(orderId) != orders.end();
    }

    void sendApiRequest(const std::string& requestJson);

public:
    OrderManager(client* clientPtr, websocketpp::connection_hdl hdl);

    bool placeOrder(const Order& order);

    bool cancelOrder(const std::string& orderId) {
       

        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "private/cancel"},
            {"params", {{"order_id", orderId}}}
        };

        sendApiRequest(requestJson.dump());
        return true;
    }

    bool modifyOrder(const std::string& orderId, const std::optional<double>& newPrice, const std::optional<double>& newAmount, std::string& advanced,bool& post_only,bool& reduce_only) {
        
        nlohmann::json editMsg = {
            {"jsonrpc", "2.0"},
            {"id", rand()},
            {"method", "private/edit"},
            {"params", {
                {"order_id", orderId},
                {"amount", newAmount.value_or(0.0)},  // Use .value_or() to handle std::optional
                {"price", newPrice.value_or(0.0)},    // Use .value_or() to handle std::optional
                {"advanced", advanced},
                {"post_only",post_only},
                {"reduce_only",reduce_only}
            }}
        };
        sendApiRequest(editMsg.dump());
        return true;
    }

    void handleApiResponse(const std::string& response) {
        auto jsonResponse = nlohmann::json::parse(response);

        if (jsonResponse.contains("result")) {
            auto result = jsonResponse["result"];
            if (result.contains("order")) {
                auto orderId = result["order"]["order_id"].get<std::string>();
                if (orderExists(orderId)) {
                    Order& order = orders[orderId];
                    order.price = result["order"]["average_price"];
                    order.amount = result["order"]["amount"];
                    std::cout << "Order " << orderId << " updated successfully.\n";
                }
            }
        }
    }

    std::optional<Order> getOrderById(const std::string& orderId) const {
        if (!orderExists(orderId)) {
            return std::nullopt;
        }
        return orders.at(orderId);
    }

    std::vector<Order> getAllOrders() const {
        std::vector<Order> allOrders;
        for (const auto& pair : orders) {
            allOrders.push_back(pair.second);
        }
        return allOrders;
    }

    std::unordered_map<std::string, double> getCurrentPositions() const {
        std::unordered_map<std::string, double> positions;
        for (const auto& pair : orders) {
            const Order& order = pair.second;
            if (order.side == "buy") {
                positions[order.instrumentName] += order.amount;
            } else if (order.side == "sell") {
                positions[order.instrumentName] -= order.amount;
            }
        }
        return positions;
    }

    bool processApiResponse(const std::string& response) {
        try {
            auto jsonResponse = nlohmann::json::parse(response);

            if (jsonResponse.contains("result")) {
                auto result = jsonResponse["result"];
                if (result.contains("order")) {
                    auto orderId = result["order"]["order_id"].get<std::string>();
                    auto orderIt = orders.find(orderId);
                    if (orderIt == orders.end()) {
                        return false;
                    }
                    Order& order = orderIt->second;

                    // Handle null values gracefully
                    auto orderTypeStr = result["order"]["order_type"].get<std::string>();
                    OrderType orderType = stringToOrderType(orderTypeStr.empty() ? "limit" : orderTypeStr); // Default to "limit" if null

                    // Update existing order
                    order.price = result["order"]["average_price"].get<double>();
                    order.amount = result["order"]["amount"].get<double>();
                    order.side = result["order"]["direction"].get<std::string>();
                    order.label = result["order"]["label"].get<std::string>();

                    // Add trades to the order
                    if (result.contains("trades")) {
                        for (const auto& tradeJson : result["trades"]) {
                            Trade trade{
                                tradeJson["trade_id"].get<std::string>(),
                                tradeJson["price"].get<double>(),
                                tradeJson["amount"].get<double>(),
                                tradeJson["fee"].get<double>(),
                                tradeJson["fee_currency"].get<std::string>(),
                                tradeJson["direction"].get<std::string>(),
                                tradeJson["timestamp"].get<int64_t>()
                            };
                            order.addTrade(trade);
                        }
                    }

                    std::cout << "Order " << orderId << " updated with new trades.\n";
                    return true;
                }
            }
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
        return false;
    }

    //  method: Get order history by currency
    void getOrderHistoryByCurrency(const std::string& currency) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "private/get_order_history_by_currency"},
            {"params", {
                {"currency", currency},
                {"count", 10}  // Number of orders to retrieve
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Get order history by instrument
    void getOrderHistoryByInstrument(const std::string& instrument) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "private/get_order_history_by_instrument"},
            {"params", {
                {"instrument_name", instrument},
                {"count", 10}  // Number of orders to retrieve
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Stream market data
    void streamMarketData(const std::string& channel) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", {channel}}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Get summary by instrument
    void getSummaryByInstrument(const std::string& instrument) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/get_book_summary_by_instrument"},
            {"params", {
                {"instrument_name", instrument}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Get summary by currency
    void getSummaryByCurrency(const std::string& currency) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/get_book_summary_by_currency"},
            {"params", {
                {"currency", currency}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Get ticker data
    void getTickerData(const std::string& instrument) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/ticker"},
            {"params", {
                {"instrument_name", instrument}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Get contract size
    void getContractSize(const std::string& instrument) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/get_instrument"},
            {"params", {
                {"instrument_name", instrument}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Get all supported currencies
    void getAllSupportedCurrencies() {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/get_currencies"}
        };
        sendApiRequest(requestJson.dump());
    }

    //  method: Subscribe to a channel
    void subscribe(const std::string& channel) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", {channel}}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    // New method: Unsubscribe from a channel
    void unsubscribe(const std::string& channel) {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/unsubscribe"},
            {"params", {
                {"channels", {channel}}
            }}
        };
        sendApiRequest(requestJson.dump());
    }

    // New method: Unsubscribe from all channels
    void unsubscribeAll() {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/unsubscribe_all"}
        };
        sendApiRequest(requestJson.dump());
    }
};