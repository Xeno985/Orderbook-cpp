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
    }
}