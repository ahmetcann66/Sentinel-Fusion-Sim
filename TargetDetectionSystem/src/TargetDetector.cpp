#include "TargetDetector.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <ranges>
#include <format>

TargetDetector::TargetDetector(double noise_thresh) 
    : next_target_id(1), noise_threshold(noise_thresh) {
    detected_targets.reserve(500);
    sensor = std::make_unique<SimpleSensor>();
}

std::string TargetDetector::threatToString(ThreatLevel level) const noexcept {
    switch (level) {
        case ThreatLevel::LOW: return "Low";
        case ThreatLevel::MEDIUM: return "Medium";
        case ThreatLevel::HIGH: return "High";
        case ThreatLevel::CRITICAL: return "Critical";
        default: return "Unknown";
    }
}

bool TargetDetector::isValidTarget(const Target& target) const noexcept {
    return target.confidence >= noise_threshold &&
           target.velocity >= 0.0 &&
           std::isfinite(target.x) && std::isfinite(target.y) && std::isfinite(target.z);
}

ThreatLevel TargetDetector::calculateThreatLevel(const Target& target) const noexcept {
    if (target.velocity > 100.0 || target.confidence > 0.9) {
        return ThreatLevel::CRITICAL;
    } else if (target.velocity > 50.0 || target.confidence > 0.7) {
        return ThreatLevel::HIGH;
    } else if (target.velocity > 25.0 || target.confidence > 0.5) {
        return ThreatLevel::MEDIUM;
    }
    return ThreatLevel::LOW;
}

std::vector<Target> TargetDetector::detectRadarTargets(const std::vector<std::vector<double>>& radar_data) {
    std::vector<Target> targets;
    targets.reserve(radar_data.size());
    
    // C++20 ranges for processing radar data
    auto processed_targets = radar_data 
        | std::views::filter([this](const auto& data_point) { 
            return data_point.size() >= 4 && data_point[3] > noise_threshold; 
          })
        | std::views::transform([this](const auto& data_point) -> Target {
            double x = data_point[0];
            double y = data_point[1]; 
            double z = data_point[2];
            double signal_strength = data_point[3];
            
            double confidence = std::min(1.0, signal_strength / 100.0);
            double velocity = std::sqrt(x*x + y*y + z*z) * 0.1;
            
            return Target(
                next_target_id++,
                x, y, z,
                velocity,
                confidence,
                calculateThreatLevel(Target{0, x, y, z, velocity, confidence, ThreatLevel::LOW}),
                "Radar detected target"
            );
        })
        | std::views::filter([this](const Target& target) { return isValidTarget(target); });
    
    std::ranges::copy(processed_targets, std::back_inserter(targets));
    
    filterNoise(targets);
    prioritizeTargets(targets);
    
    for (auto& target : targets) {
        addTarget(target);
    }
    
    return targets;
}

void TargetDetector::filterNoise(std::vector<Target>& targets) noexcept {
    // C++20 ranges version
    auto valid_range = targets 
        | std::views::filter([this](const Target& t) { return isValidTarget(t); });
    
    std::vector<Target> filtered;
    std::ranges::copy(valid_range, std::back_inserter(filtered));
    targets = std::move(filtered);
}

void TargetDetector::prioritizeTargets(std::vector<Target>& targets) noexcept {
    // C++20 three-way comparison operator usage
    std::ranges::sort(targets, std::greater<>());
}

void TargetDetector::clearTargets() noexcept {
    detected_targets.clear();
}

void TargetDetector::addTarget(const Target& target) {
    detected_targets.push_back(target);
}

std::vector<Target> TargetDetector::getDetectedTargets() const {
    return detected_targets;
}

void TargetDetector::printTargets() const {
    std::cout << "\n=== Detected Targets ===\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::setw(4) << "ID" << " | "
              << std::setw(8) << "X" << " | "
              << std::setw(8) << "Y" << " | "
              << std::setw(8) << "Z" << " | "
              << std::setw(8) << "Velocity" << " | "
              << std::setw(10) << "Confidence" << " | "
              << std::setw(8) << "Threat" << " | "
              << "Description\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& target : detected_targets) {
        std::cout << std::setw(4) << target.id << " | "
                  << std::setw(8) << target.x << " | "
                  << std::setw(8) << target.y << " | "
                  << std::setw(8) << target.z << " | "
                  << std::setw(8) << target.velocity << " | "
                  << std::setw(10) << target.confidence << " | "
                  << std::setw(8) << threatToString(target.threat_level) << " | "
                  << target.description << "\n";
    }
}

size_t TargetDetector::getTargetCount() const noexcept {
    return detected_targets.size();
}

void TargetDetector::setNoiseThreshold(double threshold) noexcept {
    noise_threshold = threshold;
}

double TargetDetector::getNoiseThreshold() const noexcept {
    return noise_threshold;
}

void TargetDetector::setSensor(std::unique_ptr<SimpleSensor> new_sensor) noexcept {
    sensor = std::move(new_sensor);
}

SimpleSensor* TargetDetector::getSensor() const noexcept {
    return sensor.get();
}

std::vector<Target> TargetDetector::detectWithSensor(double center_x, double center_y) {
    std::vector<Target> targets;
    
    if (!sensor) {
        return targets;
    }
    
    auto detections = sensor->scanForTargets(center_x, center_y);
    
    for (size_t i = 0; i < detections.size(); ++i) {
        double distance = detections[i];
        double angle = (i * 72.0) * M_PI / 180.0; // 72 degrees between 5 points
        
        double x = center_x + distance * cos(angle);
        double y = center_y + distance * sin(angle);
        double z = 0.0; // Simple 2D detection
        
        double confidence = std::max(0.5, 1.0 - distance / sensor->getRange());
        double velocity = distance * 0.05; // Simple velocity calculation
        
        Target target(
            next_target_id++,
            x, y, z,
            velocity,
            confidence,
            calculateThreatLevel(Target{0, x, y, z, velocity, confidence, ThreatLevel::LOW}),
            "Sensor detected target"
        );
        
        if (isValidTarget(target)) {
            targets.push_back(target);
            addTarget(target);
        }
    }
    
    prioritizeTargets(targets);
    return targets;
}