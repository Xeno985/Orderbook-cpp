#include<iostream>
#include "OrderManager.h"

// Menu interface
class Menu{
    public:
void displayMenu();


void placeOrderMenu(OrderManager* manager) ;

void cancelOrderMenu(OrderManager* manager) ;
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

};