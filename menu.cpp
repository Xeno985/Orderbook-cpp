#include "menu.h"

using namespace ftxui;

void Menu::showInteractiveMenu(OrderManager* manager) {
    auto screen = ScreenInteractive::Fullscreen();

    while (true) {
        std::vector<std::string> menu_entries = {
            "Place Order", "Cancel Order", "Modify Order", "View Current Positions",
            "Get Order History by Currency", "Get Order History by Instrument", "Stream Market Data",
            "Get Summary by Instrument", "Get Summary by Currency", "Get Ticker Data",
            "Get Contract Size", "Get All Supported Currencies", "Subscribe to Channel",
            "Unsubscribe from Channel", "Unsubscribe All", "Exit",
        };

        int selected = 0;
        auto menu_option = MenuOption();
        menu_option.on_enter = screen.ExitLoopClosure();
        auto menu = ftxui::Menu(&menu_entries, &selected, menu_option);

        // Handle escape key
        menu |= CatchEvent([&](Event event) {
            if (event == Event::Character('q') || event == Event::Escape) {
                selected = menu_entries.size() - 1; // Exit
                screen.ExitLoopClosure()();
                return true;
            }
            return false;
        });

        auto renderer = Renderer(menu, [&] {
            std::vector<Element> elements;
            elements.push_back(text("Trading System Menu") | bold | color(Color::Blue) | center);
            elements.push_back(separator());
            elements.push_back(menu->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 18));
            
            return vbox(elements) | border | center;
        });

        screen.Loop(renderer);

        // Process the selected option
        switch (selected) {
            case 0: placeOrderMenuFTXUI(manager); break;
            case 1: cancelOrderMenuFTXUI(manager); break;
            case 2: modifyOrderMenuFTXUI(manager); break;
            case 3: viewPositionsMenuFTXUI(manager); break;
            case 4: getOrderHistoryByCurrencyMenuFTXUI(manager); break;
            case 5: getOrderHistoryByInstrumentMenuFTXUI(manager); break;
            case 6: streamMarketDataMenuFTXUI(manager); break;
            case 7: getSummaryByInstrumentMenuFTXUI(manager); break;
            case 8: getSummaryByCurrencyMenuFTXUI(manager); break;
            case 9: getTickerDataMenuFTXUI(manager); break;
            case 10: getContractSizeMenuFTXUI(manager); break;
            case 11: getAllSupportedCurrenciesMenuFTXUI(manager); break;
            case 12: subscribeMenuFTXUI(manager); break;
            case 13: unsubscribeMenuFTXUI(manager); break;
            case 14: unsubscribeAllMenuFTXUI(manager); break;
            case 15: 
                return;
        }
    }
}

// Helper function for input dialogs
std::string Menu::showInputDialog(const std::string& title, const std::string& prompt) {
    auto screen = ScreenInteractive::Fullscreen();
    std::string input;
    bool submit = false;
    bool cancel = false;

    auto input_component = Input(&input, prompt);
    
    auto submit_button = Button("Submit", [&] { submit = true; screen.Exit(); });
    auto cancel_button = Button("Cancel", [&] { cancel = true; screen.Exit(); });

    auto layout = Container::Vertical({
        input_component,
        Container::Horizontal({
            submit_button,
            cancel_button
        })
    });

    auto renderer = Renderer(layout, [&] {
        std::vector<Element> elements;
        elements.push_back(text(title) | bold | center);
        elements.push_back(separator());
        elements.push_back(hbox(text(prompt + ": "), input_component->Render()));
        elements.push_back(separator());
        elements.push_back(hbox(submit_button->Render(), cancel_button->Render()) | center);
        
        return vbox(elements) | border | center;
    });

    screen.Loop(renderer);
    return (submit && !input.empty()) ? input : "";
}

// Helper function for message dialogs
void Menu::showMessageDialog(const std::string& message, ftxui::Color msg_color) {
    auto screen = ScreenInteractive::Fullscreen();
    auto ok_button = Button("OK", [&] { screen.Exit(); });

    auto renderer = Renderer(ok_button, [&] {
        std::vector<Element> elements;
elements.push_back(text(message) | color(msg_color) | center);
        elements.push_back(separator());
        elements.push_back(ok_button->Render() | center);
        
        return vbox(elements) | border | center;
    });

    screen.Loop(renderer);
}

// FTXUI version of placeOrderMenu
void Menu::placeOrderMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string orderId = showInputDialog("Place Order", "Order ID");
    if (orderId.empty()) return;

    std::string instrumentName = showInputDialog("Place Order", "Instrument Name");
    if (instrumentName.empty()) return;

    std::string instrTypeStr = showInputDialog("Place Order", "Instrument Type (Spot/Futures/Options)");
    if (instrTypeStr.empty()) return;

    std::string side = showInputDialog("Place Order", "Side (buy/sell)");
    if (side.empty()) return;

    std::string orderTypeStr = showInputDialog("Place Order", "Order Type (market/limit/stop_limit)");
    if (orderTypeStr.empty()) return;

    std::string amountStr = showInputDialog("Place Order", "Amount");
    if (amountStr.empty()) return;

    std::string priceStr = showInputDialog("Place Order", "Price (0 for market orders)");
    if (priceStr.empty()) return;

    std::string label = showInputDialog("Place Order", "Label");
    if (label.empty()) return;

    try {
        double amount = std::stod(amountStr);
        double price = std::stod(priceStr);
        InstrumentType instrumentType = parseInstrumentType(instrTypeStr);
        
        Order order(orderId, instrumentName, instrumentType, side, orderTypeStr, amount, price, std::nullopt, std::nullopt, label);
        manager->placeOrder(order);
        showMessageDialog("Order placed successfully!", Color::Green);
    } catch (const std::exception& e) {
        showMessageDialog("Error: " + std::string(e.what()), Color::Red);
    }
}

// FTXUI version of cancelOrderMenu
void Menu::cancelOrderMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string orderId = showInputDialog("Cancel Order", "Order ID to cancel");
    if (!orderId.empty()) {
        manager->cancelOrder(orderId);
        showMessageDialog("Order cancellation requested!", Color::Yellow);
    }
}

// FTXUI version of modifyOrderMenu
void Menu::modifyOrderMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string orderId = showInputDialog("Modify Order", "Order ID to modify");
    if (orderId.empty()) return;

    std::string amountStr = showInputDialog("Modify Order", "New Amount");
    if (amountStr.empty()) return;

    std::string priceStr = showInputDialog("Modify Order", "New Price");
    if (priceStr.empty()) return;

    std::string advanced = showInputDialog("Modify Order", "Advanced Option (uds/implv)");
    
    std::string postOnlyStr = showInputDialog("Modify Order", "Post only? (true/false)");
    if (postOnlyStr.empty()) return;

    std::string reduceOnlyStr = showInputDialog("Modify Order", "Reduce only? (true/false)");
    if (reduceOnlyStr.empty()) return;

    try {
        double newAmount = std::stod(amountStr);
        double newPrice = std::stod(priceStr);
        bool post_only = (postOnlyStr == "true" || postOnlyStr == "1");
        bool reduce_only = (reduceOnlyStr == "true" || reduceOnlyStr == "1");

        manager->modifyOrder(orderId, newPrice, newAmount, advanced, post_only, reduce_only);
        showMessageDialog("Order modification requested!", Color::Yellow);
    } catch (const std::exception& e) {
        showMessageDialog("Error: " + std::string(e.what()), Color::Red);
    }
}

// FTXUI version of viewPositionsMenu
void Menu::viewPositionsMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    auto positions = manager->getCurrentPositions();
    std::string positionsText = "Current Positions:\n";
    for (const auto& pair : positions) {
        positionsText += pair.first + ": " + std::to_string(pair.second) + "\n";
    }
    
    if (positions.empty()) {
        positionsText = "No current positions";
    }
    
    showMessageDialog(positionsText, Color::White);
}

// FTXUI version of getOrderHistoryByCurrencyMenu
void Menu::getOrderHistoryByCurrencyMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string currency = showInputDialog("Order History by Currency", "Currency (e.g., BTC, ETH)");
    if (!currency.empty()) {
        manager->getOrderHistoryByCurrency(currency);
        showMessageDialog("Order history request sent for currency: " + currency, Color::Green);
    }
}

// FTXUI version of getOrderHistoryByInstrumentMenu
void Menu::getOrderHistoryByInstrumentMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string instrument = showInputDialog("Order History by Instrument", "Instrument (e.g., BTC-PERPETUAL)");
    if (!instrument.empty()) {
        manager->getOrderHistoryByInstrument(instrument);
        showMessageDialog("Order history request sent for instrument: " + instrument, Color::Green);
    }
}

// FTXUI version of streamMarketDataMenu
void Menu::streamMarketDataMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string channel = showInputDialog("Stream Market Data", "Market Data Channel (e.g., ticker.BTC-PERPETUAL.raw)");
    if (!channel.empty()) {
        manager->streamMarketData(channel);
        showMessageDialog("Market data streaming started for: " + channel, Color::Green);
    }
}

// FTXUI version of getSummaryByInstrumentMenu
void Menu::getSummaryByInstrumentMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string instrument = showInputDialog("Summary by Instrument", "Instrument (e.g., BTC-PERPETUAL)");
    if (!instrument.empty()) {
        manager->getSummaryByInstrument(instrument);
        showMessageDialog("Summary request sent for instrument: " + instrument, Color::Green);
    }
}

// FTXUI version of getSummaryByCurrencyMenu
void Menu::getSummaryByCurrencyMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string currency = showInputDialog("Summary by Currency", "Currency (e.g., BTC, ETH)");
    if (!currency.empty()) {
        manager->getSummaryByCurrency(currency);
        showMessageDialog("Summary request sent for currency: " + currency, Color::Green);
    }
}

// FTXUI version of getTickerDataMenu
void Menu::getTickerDataMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string instrument = showInputDialog("Ticker Data", "Instrument (e.g., BTC-PERPETUAL)");
    if (!instrument.empty()) {
        manager->getTickerData(instrument);
        showMessageDialog("Ticker data request sent for: " + instrument, Color::Green);
    }
}

// FTXUI version of getContractSizeMenu
void Menu::getContractSizeMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string instrument = showInputDialog("Contract Size", "Instrument (e.g., BTC-PERPETUAL)");
    if (!instrument.empty()) {
        manager->getContractSize(instrument);
        showMessageDialog("Contract size request sent for: " + instrument, Color::Green);
    }
}

// FTXUI version of getAllSupportedCurrenciesMenu
void Menu::getAllSupportedCurrenciesMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    manager->getAllSupportedCurrencies();
    showMessageDialog("Request sent for all supported currencies", Color::Green);
}

// FTXUI version of subscribeMenu
void Menu::subscribeMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string channel = showInputDialog("Subscribe", "Channel to Subscribe (e.g., ticker.BTC-PERPETUAL.raw)");
    if (!channel.empty()) {
        manager->subscribe(channel);
        showMessageDialog("Subscribed to: " + channel, Color::Green);
    }
}

// FTXUI version of unsubscribeMenu
void Menu::unsubscribeMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    std::string channel = showInputDialog("Unsubscribe", "Channel to Unsubscribe (e.g., ticker.BTC-PERPETUAL.raw)");
    if (!channel.empty()) {
        manager->unsubscribe(channel);
        showMessageDialog("Unsubscribed from: " + channel, Color::Yellow);
    }
}

// FTXUI version of unsubscribeAllMenu
void Menu::unsubscribeAllMenuFTXUI(OrderManager* manager) {
    if (!manager) {
        showMessageDialog("Manager is null!", Color::Red);
        return;
    }

    manager->unsubscribeAll();
    showMessageDialog("Unsubscribed from all channels", Color::Yellow);
}

//original stuff cuz me too lazy
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

void Menu::getAllSupportedCurrenciesMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    manager->getAllSupportedCurrencies();
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

void Menu::unsubscribeAllMenu(OrderManager* manager) {
    if (manager == nullptr) {
        std::cerr << "Manager is null!" << std::endl;
        return;
    }

    manager->unsubscribeAll();
}