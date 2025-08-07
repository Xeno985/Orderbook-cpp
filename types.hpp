#pragma once
#include<string>
#include<stdexcept>

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
inline InstrumentType parseInstrumentType(const std::string& type) {
    if (type == "Spot") return InstrumentType::Spot;
    if (type == "Futures") return InstrumentType::Futures;
    if (type == "Options") return InstrumentType::Options;

    throw std::invalid_argument("Invalid instrument type: " + type);
}

// Helper function to convert InstrumentType to string
inline std::string instrumentTypeToString(InstrumentType type) {
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
inline std::string orderTypeToString(OrderType type) {
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
inline OrderType stringToOrderType(const std::string& str) {
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
