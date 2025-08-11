#include "OrderManager.h"

OrderManager::OrderManager(client *clientPtr, websocketpp::connection_hdl hdl)
    : wsClient(clientPtr), wsHandle(hdl) {}

bool OrderManager::placeOrder(const Order &order)
{
    if (orderExists(order.id))
    {
        std::cerr << "Order with ID " << order.id << " already exists.\n";
        return false;
    }

    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", order.side == "buy" ? "private/buy" : "private/sell"},
        {"params", {{"instrument_name", order.instrumentName}, {"amount", static_cast<int>(order.amount)}, {"type", "market"}, // Assuming limit orders; adjust as needed.
                    {"label", order.label.value_or("")}}}};
    std::cout << requestJson.dump(4) << std::endl;
    sendApiRequest(requestJson.dump());
    orders[order.id] = order; // Temporarily add the order before confirmation.
    return true;
}

void OrderManager::sendApiRequest(const std::string &requestJson)
{
    websocketpp::lib::error_code ec;
    wsClient->send(wsHandle, requestJson, websocketpp::frame::opcode::text, ec);
    if (ec)
    {
        std::cerr << "Failed to send API request: " << ec.message() << std::endl;
    }
}

bool OrderManager::cancelOrder(const std::string &orderId)
{

    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderId}}}};

    sendApiRequest(requestJson.dump());
    return true;
}
bool OrderManager::modifyOrder(const std::string &orderId, const std::optional<double> &newPrice, const std::optional<double> &newAmount, std::string &advanced, bool &post_only, bool &reduce_only)
{

    nlohmann::json editMsg = {
        {"jsonrpc", "2.0"},
        {"id", rand()},
        {"method", "private/edit"},
        {"params", {{"order_id", orderId}, {"amount", newAmount.value_or(0.0)}, // Use .value_or() to handle std::optional
                    {"price", newPrice.value_or(0.0)},                          // Use .value_or() to handle std::optional
                    {"advanced", advanced},
                    {"post_only", post_only},
                    {"reduce_only", reduce_only}}}};
    sendApiRequest(editMsg.dump());
    return true;
}

void OrderManager::handleApiResponse(const std::string &response)
{
    auto jsonResponse = nlohmann::json::parse(response);

    if (jsonResponse.contains("result"))
    {
        auto result = jsonResponse["result"];
        if (result.contains("order"))
        {
            auto orderId = result["order"]["order_id"].get<std::string>();
            if (orderExists(orderId))
            {
                Order &order = orders[orderId];
                order.price = result["order"]["average_price"];
                order.amount = result["order"]["amount"];
                std::cout << "Order " << orderId << " updated successfully.\n";
            }
        }
    }
}

void OrderManager::unsubscribeAll()
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/unsubscribe_all"}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::unsubscribe(const std::string &channel)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/unsubscribe"},
        {"params", {{"channels", {channel}}}}};
    sendApiRequest(requestJson.dump());
}

std::optional<Order> OrderManager::getOrderById(const std::string &orderId) const
{
    if (!orderExists(orderId))
    {
        return std::nullopt;
    }
    return orders.at(orderId);
}
std::vector<Order> OrderManager::getAllOrders() const
{
    std::vector<Order> allOrders;
    for (const auto &pair : orders)
    {
        allOrders.push_back(pair.second);
    }
    return allOrders;
}

std::unordered_map<std::string, double> OrderManager::getCurrentPositions() const
{
    std::unordered_map<std::string, double> positions;
    for (const auto &pair : orders)
    {
        const Order &order = pair.second;
        if (order.side == "buy")
        {
            positions[order.instrumentName] += order.amount;
        }
        else if (order.side == "sell")
        {
            positions[order.instrumentName] -= order.amount;
        }
    }
    return positions;
}

bool OrderManager::processApiResponse(const std::string &response)
{
    try
    {
        auto jsonResponse = nlohmann::json::parse(response);

        if (jsonResponse.contains("result"))
        {
            auto result = jsonResponse["result"];
            if (result.contains("order"))
            {
                auto orderId = result["order"]["order_id"].get<std::string>();
                auto orderIt = orders.find(orderId);
                if (orderIt == orders.end())
                {
                    return false;
                }
                Order &order = orderIt->second;

                // Handle null values gracefully
                auto orderTypeStr = result["order"]["order_type"].get<std::string>();
                OrderType orderType = stringToOrderType(orderTypeStr.empty() ? "limit" : orderTypeStr); // Default to "limit" if null

                // Update existing order
                order.price = result["order"]["average_price"].get<double>();
                order.amount = result["order"]["amount"].get<double>();
                order.side = result["order"]["direction"].get<std::string>();
                order.label = result["order"]["label"].get<std::string>();

                // Add trades to the order
                if (result.contains("trades"))
                {
                    for (const auto &tradeJson : result["trades"])
                    {
                        Trade trade{
                            tradeJson["trade_id"].get<std::string>(),
                            tradeJson["price"].get<double>(),
                            tradeJson["amount"].get<double>(),
                            tradeJson["fee"].get<double>(),
                            tradeJson["fee_currency"].get<std::string>(),
                            tradeJson["direction"].get<std::string>(),
                            tradeJson["timestamp"].get<int64_t>()};
                        order.addTrade(trade);
                    }
                }

                std::cout << "Order " << orderId << " updated with new trades.\n";
                return true;
            }
        }
    }
    catch (const nlohmann::json::exception &e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
    return false;
}

void OrderManager::getOrderHistoryByCurrency(const std::string &currency)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "private/get_order_history_by_currency"},
        {"params", {
                       {"currency", currency}, {"count", 10} // Number of orders to retrieve
                   }}};
    sendApiRequest(requestJson.dump());
}
void OrderManager::getOrderHistoryByInstrument(const std::string &instrument)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "private/get_order_history_by_instrument"},
        {"params", {
                       {"instrument_name", instrument}, {"count", 10} // Number of orders to retrieve
                   }}};
    sendApiRequest(requestJson.dump());
}
void OrderManager::streamMarketData(const std::string &channel)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/subscribe"},
        {"params", {{"channels", {channel}}}}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::getSummaryByInstrument(const std::string &instrument)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/get_book_summary_by_instrument"},
        {"params", {{"instrument_name", instrument}}}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::getSummaryByCurrency(const std::string &currency)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/get_book_summary_by_currency"},
        {"params", {{"currency", currency}}}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::subscribe(const std::string &channel)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/subscribe"},
        {"params", {{"channels", {channel}}}}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::getTickerData(const std::string &instrument)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/ticker"},
        {"params", {{"instrument_name", instrument}}}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::getContractSize(const std::string &instrument)
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/get_instrument"},
        {"params", {{"instrument_name", instrument}}}};
    sendApiRequest(requestJson.dump());
}

void OrderManager::getAllSupportedCurrencies()
{
    nlohmann::json requestJson = {
        {"jsonrpc", "2.0"},
        {"id", std::rand()},
        {"method", "public/get_currencies"}};
    sendApiRequest(requestJson.dump());
}