#pragma once
#include<iostream>
#include "OrderManager.h"
#include "FTXUI/include/ftxui/component/component.hpp"
#include "FTXUI/include/ftxui/component/screen_interactive.hpp"
// Menu interface

class Menu {
public:
    void showInteractiveMenu(OrderManager* manager);
    void displayMenu();  // Keep old method for compatibility
    
    // FTXUI versions
    void placeOrderMenuFTXUI(OrderManager* manager);
    void cancelOrderMenuFTXUI(OrderManager* manager);
    void modifyOrderMenuFTXUI(OrderManager* manager);
    void viewPositionsMenuFTXUI(OrderManager* manager);
    void getOrderHistoryByCurrencyMenuFTXUI(OrderManager* manager);
    void getOrderHistoryByInstrumentMenuFTXUI(OrderManager* manager);
    void streamMarketDataMenuFTXUI(OrderManager* manager);
    void getSummaryByInstrumentMenuFTXUI(OrderManager* manager);
    void getSummaryByCurrencyMenuFTXUI(OrderManager* manager);
    void getTickerDataMenuFTXUI(OrderManager* manager);
    void getContractSizeMenuFTXUI(OrderManager* manager);
    void getAllSupportedCurrenciesMenuFTXUI(OrderManager* manager);
    void subscribeMenuFTXUI(OrderManager* manager);
    void unsubscribeMenuFTXUI(OrderManager* manager);
    void unsubscribeAllMenuFTXUI(OrderManager* manager);

    // Original methods (for backward compatibility)
    void placeOrderMenu(OrderManager* manager);
    void cancelOrderMenu(OrderManager* manager);
    void modifyOrderMenu(OrderManager* manager);
    void viewPositionsMenu(OrderManager* manager);
    void getOrderHistoryByCurrencyMenu(OrderManager* manager);
    void getOrderHistoryByInstrumentMenu(OrderManager* manager);
    void streamMarketDataMenu(OrderManager* manager);
    void getSummaryByInstrumentMenu(OrderManager* manager);
    void getSummaryByCurrencyMenu(OrderManager* manager);
    void getTickerDataMenu(OrderManager* manager);
    void getContractSizeMenu(OrderManager* manager);
    void getAllSupportedCurrenciesMenu(OrderManager* manager);
    void subscribeMenu(OrderManager* manager);
    void unsubscribeMenu(OrderManager* manager);
    void unsubscribeAllMenu(OrderManager* manager);

private:
    std::string showInputDialog(const std::string& title, const std::string& prompt);
    void showMessageDialog(const std::string& message, ftxui::Color color = ftxui::Color::White);
};