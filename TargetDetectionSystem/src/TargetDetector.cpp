#define _USE_MATH_DEFINES
#include "TargetDetector.h"
#include <iostream>
#include <cmath>
#include <numbers>

TargetDetector::TargetDetector(double threshold) 
    : confidence_threshold_(threshold) {}

std::vector<Target> TargetDetector::detectRadarTargets(const std::vector<std::vector<double>>& raw_data) {
    detected_targets_.clear();

    for (const auto& signal : raw_data) {
        if (auto target = processSignal(signal)) {
            detected_targets_.push_back(*target);
        }
    }

    
    std::ranges::sort(detected_targets_, std::greater{});

    return detected_targets_;
}

std::optional<Target> TargetDetector::processSignal(const std::vector<double>& signal) {
    if (signal.size() < 4) return std::nullopt;

    double x = signal[0];
    double y = signal[1];
    double z = signal[2];
    double velocity = signal[3];

    
    double distance = std::sqrt(x*x + y*y + z*z);
    double confidence = 1.0 / (1.0 + distance * 0.001); 

    if (confidence < confidence_threshold_) {
        return std::nullopt;
    }

    ThreatLevel threat = calculateThreat(velocity, distance);
    
    return Target(
        next_target_id_++,
        x, y, z,
        velocity,
        confidence,
        threat,
        "Detected Signal"
    );
}

ThreatLevel TargetDetector::calculateThreat(double velocity, double distance) const {
    if (distance < 500.0 && std::abs(velocity) > 100.0) return ThreatLevel::CRITICAL;
    if (distance < 1000.0) return ThreatLevel::HIGH;
    if (std::abs(velocity) > 50.0) return ThreatLevel::MEDIUM;
    return ThreatLevel::LOW;
}

void TargetDetector::printTargets() const {
    std::cout << "--- Detected Targets: " << detected_targets_.size() << " ---\n";
    for (const auto& t : detected_targets_) {
        std::string threat_str;
        switch(t.threat_level) {
            case ThreatLevel::CRITICAL: threat_str = "CRITICAL"; break;
            case ThreatLevel::HIGH:     threat_str = "HIGH"; break;
            case ThreatLevel::MEDIUM:   threat_str = "MEDIUM"; break;
            case ThreatLevel::LOW:      threat_str = "LOW"; break;
        }
        
        std::cout << "ID: " << t.id 
                  << " | Threat: " << threat_str 
                  << " | Conf: " << t.confidence 
                  << " | Vel: " << t.velocity << " m/s\n";
    }
}

size_t TargetDetector::getTargetCount() const {
    return detected_targets_.size();
}