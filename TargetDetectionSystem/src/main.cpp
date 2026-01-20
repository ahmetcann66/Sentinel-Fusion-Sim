#include "TargetDetector.h"
#include <iostream>
#include <vector>
#include <random>
#include <ranges>
#include <format>

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
    std::cout << std::format("=== Radar Target Detection System ===\n\n");
    
    TargetDetector detector(0.3);
    
    // Generate sample radar data
    auto radar_data = generateRadarData(15);
    std::cout << std::format("Generated {} radar data points\n\n", radar_data.size());
    
    // Detect targets
    auto detected = detector.detectRadarTargets(radar_data);
    
    std::cout << std::format("Detection complete!\n");
    std::cout << std::format("Targets detected: {}\n", detected.size());
    std::cout << std::format("Total targets in system: {}\n", detector.getTargetCount());
    
    // Display results using ranges
    auto high_threat_targets = detected 
        | std::views::filter([](const auto& target) { 
            return target.threat_level == ThreatLevel::HIGH || 
                   target.threat_level == ThreatLevel::CRITICAL; 
          });
    
    if (!std::ranges::empty(high_threat_targets)) {
        std::cout << std::format("\nHigh priority targets: {}\n", 
                                std::ranges::distance(high_threat_targets));
    }
    
    // Display results
    detector.printTargets();
    
    return 0;
}