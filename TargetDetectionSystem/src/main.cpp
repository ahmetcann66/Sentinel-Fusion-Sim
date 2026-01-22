#include "../include/TargetDetector.h"
#include <iostream>
#include <vector>
#include <random>
#include <ranges>

#include <sstream>
#include <iomanip>

std::vector<std::vector<double>> generateRadarData(int num_targets) {
    std::vector<std::vector<double>> radar_data;
    radar_data.reserve(num_targets);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> pos_dist(-100.0, 100.0);
    std::uniform_real_distribution<double> signal_dist(10.0, 95.0);
    
    for (int i = 0; i < num_targets; ++i) {
        radar_data.emplace_back(std::vector<double>{
            pos_dist(gen),  // x
            pos_dist(gen),  // y  
            pos_dist(gen),  // z
            signal_dist(gen) // signal strength
        });
    }
    
    return radar_data;
}

int main() {
    std::cout << "=== Radar Target Detection System ===\n\n";
    
    TargetDetector detector(0.3);
    
    // Generate sample radar data
    auto radar_data = generateRadarData(15);
    std::cout << "Generated " << radar_data.size() << " radar data points\n\n";
    
    // Detect targets
    auto detected = detector.detectRadarTargets(radar_data);
    
    std::cout << "Detection complete!\n";
    std::cout << "Targets detected: " << detected.size() << "\n";
    std::cout << "Total targets in system: " << detector.getTargetCount() << "\n";
    
    // Display results using ranges
    auto high_threat_targets = detected 
        | std::views::filter([](const auto& target) { 
            return target.threat_level == ThreatLevel::HIGH || 
                   target.threat_level == ThreatLevel::CRITICAL; 
          });
    
    if (!std::ranges::empty(high_threat_targets)) {
        std::cout << "\nHigh priority targets: " << std::ranges::distance(high_threat_targets) << "\n";
    }
    
    // Display results
    detector.printTargets();
    
    return 0;
}