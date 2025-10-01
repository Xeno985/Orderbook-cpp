#pragma once
#include "OrderManager.h"
#include "FTXUI/include/ftxui/component/component.hpp"
#include "FTXUI/include/ftxui/component/screen_interactive.hpp"
#include<vector>
#include<string>
#include<map>
#include "FTXUI/include/ftxui/dom/elements.hpp"
//position object to bundle params together
struct Position{
    std::string instrument;
    double amount;
    double entry_price;
    double current_price;
    double pnl;
    double pnl_percent;
};

class PortfolioTracker{
public:
    PortfolioTracker(OrderManager* manager);
    void Run();


private:
    OrderManager *_manager;
    ftxui::Component CreatePortfolioTracker();
    ftxui::Component CreateOrderPanel();
    ftxui::Component CreateMainLayout();

    std::vector<std::vector<std::string>> GetPortfolioData();
    double CalculatePnL();
    
};

void StartTradingInterface(OrderManager* manager);
