#include "menu.h"


void Menu::showInteractiveMenu(OrderManager* manager) {
    using namespace ftxui;
    auto screen = ScreenInteractive::TerminalOutput();

    while (true) {
        std::vector<std::string> menu_entries = {
            "Place Order", "Cancel Order", "Modify Order", "View Current Positions",
            "Get Order History by Currency", "Get Order History by Instrument", "Stream Market Data",
            "Get Summary by Instrument", "Get Summary by Currency", "Get Ticker Data",
            "Get Contract Size", "Get All Supported Currencies", "Subscribe to Channel",
            "Unsubscribe from Channel", "Unsubscribe All", "Exit",
        };

        int selected = 0;
        auto menu_component = Menu(&menu_entries, &selected);

        auto renderer = Renderer(menu_component, [&] {
            return vbox({
                       text("Trading System Menu") | bold | color(Color::Blue) | hcenter,
                       separator(),
                       menu_component->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 18)
                   }) |
                   border | center;
        });
        
        // This makes the menu component exit when enter is pressed.
        renderer |= CatchEvent([&](Event event) {
            if (event == Event::Character('q')) {
                screen.ExitLoopClosure()();
                selected = menu_entries.size() - 1; // Set to "Exit"
                return true;
            }
            return false;
        });

        screen.Loop(renderer);

        // After the user selects an option, the screen loop exits and we land here.
        // We use the 'selected' variable to decide which function to call.
        
        // Clear the screen before calling the std::cin based function
        system("clear || cls");

        switch (selected) {
            case 0: placeOrderMenu(manager); break;
            case 1: cancelOrderMenu(manager); break;
            case 2: modifyOrderMenu(manager); break;
            case 3: viewPositionsMenu(manager); break;
            case 4:  getOrderHistoryByCurrencyMenu(manager);  break;
            case 5: getOrderHistoryByInstrumentMenu(manager); break;
            case 6: streamMarketDataMenu(manager); break;
            case 7: getSummaryByInstrumentMenu(manager); break;
            case 8: getSummaryByCurrencyMenu(manager); break;
            case 9: getTickerDataMenu(manager); break;
            case 10: getContractSizeMenu(manager); break;
            case 11: getAllSupportedCurrenciesMenu(manager); break;
            case 12: subscribeMenu(manager); break;
            case 13: unsubscribeMenu(manager); break;
            case 14: unsubscribeAllMenu(manager); break;
            case 15: // Exit
                std::cout << "Exiting program.\n";
                return; // Exit the while(true) loop and the function
        }
        // If we don't return, the while(true) loop continues, re-displaying the menu.
    }
}


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

void Menu::unsubscribeAllMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    manager->unsubscribeAll();
}

void Menu::unsubscribeMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string channel;
    std::cout << "Enter Channel to Unsubscribe (e.g., ticker.BTC-PERPETUAL.raw): ";
    std::cin >> channel;
    manager->unsubscribe(channel);
}

void Menu::subscribeMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string channel;
    std::cout << "Enter Channel to Subscribe (e.g., ticker.BTC-PERPETUAL.raw): ";
    std::cin >> channel;
    manager->subscribe(channel);
}

void Menu::getAllSupportedCurrenciesMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    manager->getAllSupportedCurrencies();
}

void Menu::getContractSizeMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getContractSize(instrument);
}

void Menu::getTickerDataMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getTickerData(instrument);
}

void Menu::getSummaryByCurrencyMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string currency;
    std::cout << "Enter Currency (e.g., BTC, ETH): ";
    std::cin >> currency;
    manager->getSummaryByCurrency(currency);
}

void Menu::getSummaryByInstrumentMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getSummaryByInstrument(instrument);
}

void Menu::streamMarketDataMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string channel;
    std::cout << "Enter Market Data Channel (e.g., ticker.BTC-PERPETUAL.raw): ";
    std::cin >> channel;
    manager->streamMarketData(channel);
}

void Menu::getOrderHistoryByInstrumentMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string instrument;
    std::cout << "Enter Instrument (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument;
    manager->getOrderHistoryByInstrument(instrument);
}

void Menu::getOrderHistoryByCurrencyMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    std::string currency;
    std::cout << "Enter Currency (e.g., BTC, ETH): ";
    std::cin >> currency;
    manager->getOrderHistoryByCurrency(currency);
}
void Menu::viewPositionsMenu(OrderManager* manager) {
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
void Menu::modifyOrderMenu(OrderManager* manager) {
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

