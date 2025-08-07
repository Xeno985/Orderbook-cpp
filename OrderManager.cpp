#include "OrderManager.h"

    OrderManager::OrderManager(client* clientPtr, websocketpp::connection_hdl hdl)
        : wsClient(clientPtr), wsHandle(hdl) {}



bool OrderManager::placeOrder(const Order& order) {
        if (orderExists(order.id)) {
            std::cerr << "Order with ID " << order.id << " already exists.\n";
            return false;
        }

        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", order.side == "buy" ? "private/buy" : "private/sell"},
            {"params", {
                {"instrument_name", order.instrumentName},
                {"amount", static_cast<int>(order.amount)},
                {"type", "market"},  // Assuming limit orders; adjust as needed.
                {"label", order.label.value_or("")}
            }}
        };
        std::cout << requestJson.dump(4) << std::endl;
        sendApiRequest(requestJson.dump());
        orders[order.id] = order;  // Temporarily add the order before confirmation.
        return true;
    }

    
    void OrderManager::sendApiRequest(const std::string& requestJson) {
        websocketpp::lib::error_code ec;
        wsClient->send(wsHandle, requestJson, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cerr << "Failed to send API request: " << ec.message() << std::endl;
        }
    }

        bool OrderManager::cancelOrder(const std::string& orderId) {
       

        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "private/cancel"},
            {"params", {{"order_id", orderId}}}
        };

        sendApiRequest(requestJson.dump());
        return true;
    }
    bool OrderManager::modifyOrder(const std::string& orderId, const std::optional<double>& newPrice, const std::optional<double>& newAmount, std::string& advanced,bool& post_only,bool& reduce_only) {
        
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

        void OrderManager::handleApiResponse(const std::string& response) {
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

        void OrderManager::unsubscribeAll() {
        nlohmann::json requestJson = {
            {"jsonrpc", "2.0"},
            {"id", std::rand()},
            {"method", "public/unsubscribe_all"}
        };
        sendApiRequest(requestJson.dump());
    }

    void OrderManager::unsubscribe(const std::string& channel) {
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