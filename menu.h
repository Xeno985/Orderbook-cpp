#include<iostream>
#include "OrderManager.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
// Menu interface
class Menu{
    public:
    void showInteractiveMenu(OrderManager* manager);
void displayMenu();


void placeOrderMenu(OrderManager* manager) ;

void cancelOrderMenu(OrderManager* manager) ;
void modifyOrderMenu(OrderManager* manager) ;
void viewPositionsMenu(OrderManager* manager) ;

void getOrderHistoryByCurrencyMenu(OrderManager* manager);

void getOrderHistoryByInstrumentMenu(OrderManager* manager);

void streamMarketDataMenu(OrderManager* manager) ;

void getSummaryByInstrumentMenu(OrderManager* manager);

void getSummaryByCurrencyMenu(OrderManager* manager) ;

void getTickerDataMenu(OrderManager* manager) ;
void getContractSizeMenu(OrderManager* manager);

void getAllSupportedCurrenciesMenu(OrderManager* manager);

void subscribeMenu(OrderManager* manager);

void unsubscribeMenu(OrderManager* manager);

void unsubscribeAllMenu(OrderManager* manager);

};