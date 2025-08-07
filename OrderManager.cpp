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