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

// Authenticator class
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
    Authenticator() {
        auth_message = R"({
            "jsonrpc": "2.0",
            "id": 9929,
            "method": "public/auth",
            "params": {
                "grant_type": "client_credentials",
                "client_id": ")" + client_id + R"(",
                "client_secret": ")" + client_secret + R"("
            }
        })";
    }

    void send_authcall(client* c, websocketpp::connection_hdl hdl) {
        websocketpp::lib::error_code ec;
        c->send(hdl, auth_message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "Error sending authentication message: " << ec.message() << std::endl;
        }
    }
};

// OrderType enum
enum class OrderType {
    limit,
    stop_limit,
    take_limit,
    market,
    stop_market,
    take_market,
    market_limit,
    trailing_stop
};

// InstrumentType enum
enum class InstrumentType {
    Spot,
    Futures,
    Options
};

// Parse InstrumentType
InstrumentType parseInstrumentType(const std::string& type) {
    if (type == "Spot") return InstrumentType::Spot;
    if (type == "Futures") return InstrumentType::Futures;
    if (type == "Options") return InstrumentType::Options;

    throw std::invalid_argument("Invalid instrument type: " + type);
}

// Helper function to convert InstrumentType to string
std::string instrumentTypeToString(InstrumentType type) {
    switch (type) {
        case InstrumentType::Spot: return "Spot";
        case InstrumentType::Futures: return "Futures";
        case InstrumentType::Options: return "Options";
        default: throw std::invalid_argument("Unknown instrument type");
    }
}

// Trade struct
struct Trade {
    std::string tradeId;       // Trade ID
    double price;              // Execution price
    double amount;             // Executed amount
    double fee;                // Fee charged
    std::string feeCurrency;   // Fee currency
    std::string direction;     // Buy or sell
    int64_t timestamp;         // Execution timestamp
};

// OrderType to string
std::string orderTypeToString(OrderType type) {
    switch (type) {
        case OrderType::limit: return "limit";
        case OrderType::stop_limit: return "stop_limit";
        case OrderType::take_limit: return "take_limit";
        case OrderType::market: return "market";
        case OrderType::stop_market: return "stop_market";
        case OrderType::take_market: return "take_market";
        case OrderType::market_limit: return "market_limit";
        case OrderType::trailing_stop: return "trailing_stop";
        default: throw std::invalid_argument("Unknown order type");
    }
}

// String to OrderType
OrderType stringToOrderType(const std::string& str) {
    if (str == "limit") return OrderType::limit;
    if (str == "stop_limit") return OrderType::stop_limit;
    if (str == "take_limit") return OrderType::take_limit;
    if (str == "market") return OrderType::market;
    if (str == "stop_market") return OrderType::stop_market;
    if (str == "take_market") return OrderType::take_market;
    if (str == "market_limit") return OrderType::market_limit;
    if (str == "trailing_stop") return OrderType::trailing_stop;

    throw std::invalid_argument("Invalid order type: " + str);
}

// Order class
class Order {
private:
    std::string id;                        // Unique order identifier
    std::string instrumentName;            // Instrument name (e.g., ETH-PERPETUAL)
    std::string side;                      // Order side: "buy" or "sell"
    InstrumentType instrumentType;         // Instrument type: Spot, Futures, or Options
    OrderType type;                        // Order type: "market", "limit", "stop_limit", etc.
    double amount;                         // Order amount
    std::optional<double> price;           // Price (for limit or stop_limit orders)
    std::optional<double> triggerPrice;    // Trigger price (for stop_limit orders)
    std::optional<std::string> trigger;    // Trigger type: "last_price", "mark_price", etc.
    std::optional<std::string> label;      // Optional order label
    std::vector<Trade> trades;  

    static std::vector<std::string> tokenize(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    Order(
        const std::string& orderId,
        const std::string& instrument,
        InstrumentType instrType,
        const std::string& orderSide,
        const std::string& orderTypeStr,  // Change to string
        double orderAmount,
        std::optional<double> orderPrice = std::nullopt,
        std::optional<double> orderTriggerPrice = std::nullopt,
        std::optional<std::string> orderTrigger = std::nullopt,
        std::optional<std::string> orderLabel = std::nullopt
    ) : id(orderId),
        instrumentName(instrument),
        instrumentType(instrType),
        side(orderSide),
        type(stringToOrderType(orderTypeStr)),  // Use the existing conversion function
        amount(orderAmount),
        price(orderPrice),
        triggerPrice(orderTriggerPrice),
        trigger(orderTrigger),
        label(orderLabel) {}

    Order() = default; // Default constructor

    void addTrade(const Trade& trade) {
        trades.push_back(trade);
    }

    friend class OrderManager;

    static Order fromSimpleString(const std::string& input) {
        std::vector<std::string> tokens = tokenize(input, ' ');

        if (tokens.size() < 8) {
            std::cerr << "Invalid input string.\n";
            throw std::invalid_argument("Insufficient parameters in input.");
        }

        std::string orderId = tokens[0];
        std::string instrumentName = tokens[1];
        InstrumentType type;

        if (tokens[2] == "Futures") type = InstrumentType::Futures;
        else if (tokens[2] == "Spot") type = InstrumentType::Spot;
        else if (tokens[2] == "Options") type = InstrumentType::Options;
        else throw std::invalid_argument("Invalid instrument type.");

        std::string side = tokens[3];
        std::string orderType = tokens[4];
        int amount = std::stod(tokens[5]);
        std::optional<double> price = tokens[6] == "0" ? std::optional<double>{} : std::stod(tokens[6]);
        std::string label = tokens[7];

        return Order(
            orderId,
            instrumentName,
            type,
            side,
            orderType,
            amount,
            price,
            std::nullopt,  // triggerPrice
            std::nullopt,  // trigger
            tokens.size() > 7 ? std::optional<std::string>(tokens[7]) : std::nullopt  // label
        );
    }

    const std::string& getInstrumentName() const { return instrumentName; }
    double getAmount() const { return amount; }
    const std::optional<double>& getPrice() const { return price; }
    const std::optional<std::string> getLabel() const { return label; }
};

// OrderManager class
class OrderManager {
private:
    std::unordered_map<std::string, Order> orders;
    client* wsClient;
    websocketpp::connection_hdl wsHandle;

    bool orderExists(const std::string& orderId) const {
        return orders.find(orderId) != orders.end();
    }

    void sendApiRequest(const std::string& requestJson) {
        websocketpp::lib::error_code ec;
        wsClient->send(wsHandle, requestJson, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cerr << "Failed to send API request: " << ec.message() << std::endl;
        }
    }

public:
    OrderManager(client* clientPtr, websocketpp::connection_hdl hdl)
        : wsClient(clientPtr), wsHandle(hdl) {}

    bool placeOrder(const Order& order) {
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