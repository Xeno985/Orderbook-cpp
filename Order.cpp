#include "Order.h"
#include<iostream>
#include<stdexcept>
#include "types.hpp"

Order Order::fromSimpleString(const std::string& input){
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

     std::vector<std::string> Order::tokenize(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
       Order::Order(
        const std::string& orderId,
        const std::string& instrument,
        InstrumentType instrType,
        const std::string& orderSide,
        const std::string& orderTypeStr,  // Change to string
        double orderAmount,
        std::optional<double> orderPrice,
        std::optional<double> orderTriggerPrice,
        std::optional<std::string> orderTrigger,
        std::optional<std::string> orderLabel
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