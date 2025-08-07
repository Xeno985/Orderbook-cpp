#pragma once
#include<string>
#include<vector>
#include<optional>
#include<sstream>
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

    }

public:
 
Order(
        const std::string& orderId,
        const std::string& instrument,
        InstrumentType instrType,
        const std::string& orderSide,
        const std::string& orderTypeStr,
        double orderAmount,
        std::optional<double> orderPrice = std::nullopt,
        std::optional<double> orderTriggerPrice = std::nullopt,
        std::optional<std::string> orderTrigger = std::nullopt,
        std::optional<std::string> orderLabel = std::nullopt
    );

    Order() = default; // Default constructor

    void addTrade(const Trade& trade) {
        trades.push_back(trade);
    }

    friend class OrderManager;

    static Order fromSimpleString(const std::string& input) {}

    const std::string& getInstrumentName() const { return instrumentName; }
    double getAmount() const { return amount; }
    const std::optional<double>& getPrice() const { return price; }
    const std::optional<std::string> getLabel() const { return label; }
};