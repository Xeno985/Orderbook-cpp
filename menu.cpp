#include "menu.h"

void Menu::displayMenu() {
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

void Menu::placeOrderMenu(OrderManager* manager) {
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

void Menu::cancelOrderMenu(OrderManager *manager){
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string orderId;
    std::cout << "Enter Order ID to cancel: ";
    std::cin >> orderId;
    manager->cancelOrder(orderId);
}

