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
    );

    return Renderer(mainLayout,[&]{
        return hbox(
            {
                portfolioTracker-> Render()|flex,
                separator(),
                orderPanel-> Render()|flex
            }
        )|border;
    }
);

}

ftxui::Component PortfolioTracker::CreatePortfolioTracker(){
    return Renderer(
        [&]{
            if(!_manager){
                return text("OrderManager not initialized")|color(ftxui::Color::Red)|center;
            }

            auto positions= _manager-> getCurrentPositions();
        }
        ftxui::Elements rows;
    )
}