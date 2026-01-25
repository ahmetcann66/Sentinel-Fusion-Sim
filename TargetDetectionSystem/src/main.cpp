#include <iostream>
#include <vector>
#include <random>
#include "../include/TargetDetector.h"


std::vector<std::vector<double>> generateMockData(int count) {
    std::vector<std::vector<double>> data;
    std::mt19937 gen(42); 
    std::uniform_real_distribution<> pos_dist(0.0, 2000.0); 
    std::uniform_real_distribution<> vel_dist(-300.0, 300.0); 

    for(int i = 0; i < count; ++i) {
        data.push_back({
            pos_dist(gen), 
            pos_dist(gen), 
            pos_dist(gen), 
            vel_dist(gen)  
        });
    }
    return data;
}

int main() {
    std::cout << "--- Radar System Initializing ---\n";

    
    auto raw_data = generateMockData(10);
    

    TargetDetector detector(0.3);

   
    std::cout << "Processing " << raw_data.size() << " signals...\n";
    auto targets = detector.detectRadarTargets(raw_data);

    
    detector.printTargets();

    std::cout << "\nTotal Targets Detected: " << detector.getTargetCount() << "\n";

    return 0;
}