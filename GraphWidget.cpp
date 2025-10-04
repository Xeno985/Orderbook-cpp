#include "GraphWidget.h"
#include <algorithm>
#include<numeric>

std::vector<int> GraphUtils::NormalizeData(const std::vector<double>& data, int height){
    std::vector<int> normalized;
    if(data.empty()) return normalized;

    double minVal=FindMin(data);
    double maxVal=FindMax(data);
    double range=maxVal - minVal;

    if(range==0) range=1; // Prevent division by zero
    for(auto value:data){
        double normalizedValue=(value - minVal) / range;
        int scaled_Val=static_cast<int>(normalizedValue * (height - 1)+0.5);
        normalized.push_back(scaled_Val);
    }
    return normalized;

    
}


double GraphUtils::FindMin(const std::vector<double>& data){
    if(data.empty()) return 0.0;
    return *std::min_element(data.begin(),data.end());
}

double GraphUtils::FindMax(const std::vector<double>& data){
    if(data.empty()) return 0.0;
    return *std::max_element(data.begin(),data.end());
}

ftxui::Element GraphUtils::PlotLineGraph(const std::vector<double> &data,const std::string &title,int width, int height){
    auto graph_func=[data,height](int graph_width)->std::vector<int>{
        if(data.empty()) return {};

        std::vector<double> sampled_data;

        if(data.size()<= static_cast<size_t>(graph_width)){
            sampled_data=data;

        }
        else{
            double step=static_cast<double>(data.size())/graph_width;
            for(int i=0;i<graph_width;++i){
                size_t index=static_cast<size_t>(i*step);
                sampled_data.push_back(data[index]);
            }
        }
        return NormalizeData(sampled_data,height);
    };

    ftxui::Element graph=ftxui::graph(graph_func)|color(ftxui::Color::Cyan);

    if(!title.empty()){
        return vbox(
            {
                text(title)|bold|center,
                separator(),
                graph
            }
        );
    }
    return graph;
}

Element GraphUtils::PlotBarGraph(const std::vector<std::pair<std::string, double>>& data,
                                const std::string& title){


    ftxui::Element bars;        
    
    if(!title.empty()){
        bars.push_back(text(title)|bold|center);
        bars.push_back(separator());
    }

    if(data.empty()){
        bars.push_back(text("No data available")|center);
        return vbos(bars)|border;
    }

    double max_val=0;
    for const auto& pair:data){
        if(pair.second>max_val) max_val=pair.second;
    }

    if(max_val==0) max_val=1;

    for(const auto& [label,value]:data){
        double ratio=value/max_val;
        bars.push_back(
            hbox(
                {
                    text(label+":")|flex(1),
                    gauge(ratio)|color(Color::Green)|flex(3),
                    text(" "+std::to_string(static_cast<int>(value)))|flex(1)
                }
            )
        );
    }
        
    return vbox(bars)|border;
        
                        
}


ftxui::Element GraphUtils::PlotPriceMovement(const std::vector<double>& prices,const std::string& title){
    if(prices.empty()){
        return text("No price data available")|center|border;
    }

    double curr=prices.back();
    double min_price=FindMin(prices);
    double max_price=FindMax(prices);

    auto graph=PlotLineGraph(prices,title);

    //now to add some more price info

    return vbox({
        graph,
        separator(),
        hbox({
            text("Current: $"+std::to_string(static_cast<int>(curr)))|flex(1),
            text("Min: $"+std::to_string(static_cast<int>(min_price)))|flex(1),
            text("Max: $"+std::to_string(static_cast<int>(max_price)))| flex(1)
        })
    })|border;
}