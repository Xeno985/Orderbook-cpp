#pragma once

#include<vector>
#include<string>
#include<map>
#include<cmath>
#include "FTXUI/include/ftxui/component/component.hpp"
#include "FTXUI/include/ftxui/dom/elements.hpp"

class GraphUtils{

    static ftxui::Element PlotLineGraph(const std::vector<double>& data,const std::string& title="",int width=50,int height=10);

    static ftxui::Element PlotBarGraph(const std::vector<std::pair<std::string,double>>& data,const std::string& title="");

    static ftxui::Element PlotPriceMovement(const std::vector<double>& prices,const std::string& title="");
};