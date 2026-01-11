#include "TargetDetector.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>

TargetDetector::TargetDetector(double fusion_thresh, double noise_thresh) 
    : next_target_id(1), fusion_threshold(fusion_thresh), noise_threshold(noise_thresh) {}

double TargetDetector::calculateDistance(const Target& t1, const Target& t2) {
    return sqrt(pow(t1.x - t2.x, 2) + pow(t1.y - t2.y, 2) + pow(t1.z - t2.z, 2));
}

bool TargetDetector::isValidTarget(double confidence, double size) {
    return confidence > 0.5 && size > 0.1;
}

std::vector<Target> TargetDetector::detectRadarTargets(const std::vector<std::vector<double>>& radar_data) {
    std::vector<Target> targets;
    
    for (const auto& reading : radar_data) {
        if (reading.size() >= 4) {
            double x = reading[0];
            double y = reading[1];
            double z = reading[2];
            double signal_strength = reading[3];
            
            if (signal_strength > 0.3) {
                Target target;
                target.id = next_target_id++;
                target.x = x;
                target.y = y;
                target.z = z;
                target.velocity = std::sqrt(x*x + y*y) * 0.1;
                target.size = signal_strength * 2.0;
                target.confidence = std::min(signal_strength * 1.5, 1.0);
                target.type = "Radar";
                
                if (isValidTarget(target.confidence, target.size)) {
                    targets.push_back(target);
                }
            }
        }
    }
    
    return targets;
}

std::vector<Target> TargetDetector::detectThermalTargets(const std::vector<std::vector<double>>& thermal_data) {
    std::vector<Target> targets;
    
    for (const auto& reading : thermal_data) {
        if (reading.size() >= 4) {
            double x = reading[0];
            double y = reading[1];
            double z = reading[2];
            double temperature = reading[3];
            
            if (temperature > 25.0) {
                Target target;
                target.id = next_target_id++;
                target.x = x;
                target.y = y;
                target.z = z;
                target.velocity = std::sqrt(x*x + y*y) * 0.05;
                target.size = std::max((temperature - 20.0) * 0.3, 0.5);
                target.confidence = std::min((temperature - 20.0) / 20.0, 1.0);
                target.type = "Thermal";
                
                if (isValidTarget(target.confidence, target.size)) {
                    targets.push_back(target);
                }
            }
        }
    }
    
    return targets;
}

std::vector<Target> TargetDetector::detectOpticalTargets(const std::vector<std::vector<double>>& optical_data) {
    std::vector<Target> targets;
    
    for (const auto& reading : optical_data) {
        if (reading.size() >= 5) {
            double x = reading[0];
            double y = reading[1];
            double z = reading[2];
            double brightness = reading[3];
            double contrast = reading[4];
            
            double optical_confidence = brightness * contrast;
            
            if (optical_confidence > 0.2) {
                Target target;
                target.id = next_target_id++;
                target.x = x;
                target.y = y;
                target.z = z;
                target.velocity = std::sqrt(x*x + y*y) * 0.08;
                target.size = brightness * 3.0;
                target.confidence = std::min(optical_confidence * 2.0, 1.0);
                target.type = "Optical";
                
                if (isValidTarget(target.confidence, target.size)) {
                    targets.push_back(target);
                }
            }
        }
    }
    
    return targets;
}

void TargetDetector::filterNoise(std::vector<Target>& targets) {
    targets.erase(
        std::remove_if(targets.begin(), targets.end(),
                      [](const Target& t) { return t.confidence < 0.3 || t.size < 0.2; }),
        targets.end()
    );
}

void TargetDetector::trackTargets(std::vector<Target>& current_targets) {
    for (auto& target : current_targets) {
        double noise_x = (rand() % 100 - 50) / 100.0;
        double noise_y = (rand() % 100 - 50) / 100.0;
        
        target.x += noise_x;
        target.y += noise_y;
        
        target.velocity = std::sqrt(target.x*target.x + target.y*target.y) * 0.1;
    }
}

std::vector<Target> TargetDetector::fuseSensors(const std::vector<Target>& radar_targets,
                                               const std::vector<Target>& thermal_targets,
                                               const std::vector<Target>& optical_targets) {
    std::vector<Target> fused_targets;
    
    std::vector<Target> all_targets = radar_targets;
    all_targets.insert(all_targets.end(), thermal_targets.begin(), thermal_targets.end());
    all_targets.insert(all_targets.end(), optical_targets.begin(), optical_targets.end());
    
    for (const auto& target : all_targets) {
        bool found_similar = false;
        
        for (auto& fused : fused_targets) {
            double distance = calculateDistance(target, fused);
            
            if (distance < 5.0) {
                fused.confidence = std::min(fused.confidence + target.confidence * 0.3, 1.0);
                fused.size = (fused.size + target.size) / 2.0;
                fused.type += "+" + target.type;
                found_similar = true;
                break;
            }
        }
        
        if (!found_similar) {
            fused_targets.push_back(target);
        }
    }
    
    return fused_targets;
}

void TargetDetector::clearTargets() {
    detected_targets.clear();
    next_target_id = 1;
}

std::vector<Target> TargetDetector::getDetectedTargets() const {
    return detected_targets;
}

void TargetDetector::printTargets() const {
    std::cout << "\n=== DETECTED TARGETS ===" << std::endl;
    std::cout << std::setw(4) << "ID" << " | "
              << std::setw(8) << "Type" << " | "
              << std::setw(8) << "X" << " | "
              << std::setw(8) << "Y" << " | "
              << std::setw(8) << "Z" << " | "
              << std::setw(8) << "Vel" << " | "
              << std::setw(8) << "Size" << " | "
              << std::setw(10) << "Conf" << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    for (const auto& target : detected_targets) {
        std::cout << std::setw(4) << target.id << " | "
                  << std::setw(8) << target.type << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.x << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.y << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.z << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.velocity << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.size << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.confidence * 100 << "%" 
                  << std::endl;
    }
    std::cout << std::endl;
}