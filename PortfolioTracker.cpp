#include "PortfolioTracker.h"

PortfolioTracker::PortfolioTracker(OrderManager* manager):_manager(manager{

}

void PortfolioTracker::Run(){
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto mainLayout = CreateMainLayout();
    screen.Loop(mainLayout);
}

ftxui::Component PortfolioTracker::CreateMainLayout(){

    auto portfolioTracker=CreatePortfolioTracker();
    auto orderPanel=CreateOrderPanel();

    auto mainLayout=ftxui::Container::Horizontal(
        {
            portfolioTracker,
            orderPanel
        }
    )
}