#include "TargetDetector.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <map>
#include <random>

std::string TargetDetector::typeToString(TargetType type) {
    switch (type) {
        case TargetType::RADAR: return "Radar";
        case TargetType::THERMAL: return "Thermal";
        case TargetType::OPTICAL: return "Optical";
        case TargetType::FUSED: return "Fused";
        default: return "Unknown";
    }
}

std::string TargetDetector::threatToString(ThreatLevel level) {
    switch (level) {
        case ThreatLevel::LOW: return "Low";
        case ThreatLevel::MEDIUM: return "Medium";
        case ThreatLevel::HIGH: return "High";
        case ThreatLevel::CRITICAL: return "Critical";
        default: return "Unknown";
    }
}

TargetDetector::TargetDetector(double fusion_thresh, double noise_thresh) 
    : next_target_id(1), fusion_threshold(fusion_thresh), noise_threshold(noise_thresh) {
    target_history.reserve(1000); // Pre-allocate for better performance
    detected_targets.reserve(500);
}

double TargetDetector::calculateDistance(const Target& t1, const Target& t2) {
    return sqrt(pow(t1.x - t2.x, 2) + pow(t1.y - t2.y, 2) + pow(t1.z - t2.z, 2));
}

bool TargetDetector::isValidTarget(const Target& target) {
    return target.confidence > 0.5 && target.size > 0.1;
}

std::vector<Target> TargetDetector::detectRadarTargets(const std::vector<std::vector<double>>& radar_data) {
    std::vector<Target> targets;
    
    if (radar_data.empty()) {
        std::cerr << "Uyarı: Radar verisi boş" << std::endl;
        return targets;
    }
    
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
                target.type = TargetType::RADAR;
                
if (isValidTarget(target)) {
                    targets.push_back(target);
                }
            }
        }
    }
    
    return targets;
}

std::vector<Target> TargetDetector::detectThermalTargets(const std::vector<std::vector<double>>& thermal_data) {
    std::vector<Target> targets;
    
    if (thermal_data.empty()) {
        std::cerr << "Uyarı: Termal verisi boş" << std::endl;
        return targets;
    }
    
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
                target.type = TargetType::THERMAL;
                
if (isValidTarget(target)) {
                    targets.push_back(target);
                }
            }
        }
    }
    
    return targets;
}

std::vector<Target> TargetDetector::detectOpticalTargets(const std::vector<std::vector<double>>& optical_data) {
    std::vector<Target> targets;
    
    if (optical_data.empty()) {
        std::cerr << "Uyarı: Optik verisi boş" << std::endl;
        return targets;
    }
    
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
                target.type = TargetType::OPTICAL;
                
if (isValidTarget(target)) {
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

void TargetDetector::trackTargets(std::vector<Target>& current_targets, double time_delta) {
    // Use member variable instead of static for better memory management
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    thread_local std::normal_distribution<double> noise(0.0, 0.1);
    
    for (auto& target : current_targets) {
        // Try to find corresponding previous target
        auto it = target_history.find(target.id);
        
        if (it != target_history.end()) {
            const Target& prev = it->second;
            
            // Update velocity based on movement
            updateTargetVelocity(target, time_delta);
            
            // Optimized distance calculation
            double prev_dist_xy = sqrt(prev.x*prev.x + prev.y*prev.y + 0.001);
            double prev_dist_xyz = sqrt(prev.x*prev.x + prev.y*prev.y + prev.z*prev.z + 0.001);
            
            // Predicted position based on previous velocity
            double pred_x = prev.x + prev.velocity * (prev.x / prev_dist_xy) * time_delta;
            double pred_y = prev.y + prev.velocity * (prev.y / prev_dist_xy) * time_delta;
            double pred_z = prev.z + prev.velocity * (prev.z / prev_dist_xyz) * time_delta;
            
            // Kalman-like filtering (simple alpha-beta filter)
            const double alpha = 0.3;  // Position filter gain
            const double beta = 0.1;   // Velocity filter gain
            
            // Update position with prediction
            target.x = alpha * target.x + (1 - alpha) * pred_x;
            target.y = alpha * target.y + (1 - alpha) * pred_y;
            target.z = alpha * target.z + (1 - alpha) * pred_z;
            
            // Add small realistic noise
            target.x += noise(gen);
            target.y += noise(gen);
            target.z += noise(gen);
            
            // Update threat level based on movement
            target.threat_level = calculateThreatLevel(target);
            
        } else {
            // New target, initialize velocity if not set
            if (target.velocity <= 0) {
                target.velocity = sqrt(target.x*target.x + target.y*target.y) * 0.1;
            }
            target.threat_level = calculateThreatLevel(target);
        }
        
        // Store current target for next iteration
        target_history[target.id] = target;
    }
    
    // Remove old targets (simple cleanup)
    if (target_history.size() > 1000) {
        target_history.clear();
        target_history.reserve(1000);
    }
}

std::vector<Target> TargetDetector::fuseSensors(const std::vector<Target>& radar_targets,
                                                const std::vector<Target>& thermal_targets,
                                                const std::vector<Target>& optical_targets) {
    std::vector<Target> fused_targets;
    fused_targets.reserve(radar_targets.size() + thermal_targets.size() + optical_targets.size());
    
    // Pre-allocate and move all targets
    std::vector<Target> all_targets;
    all_targets.reserve(radar_targets.size() + thermal_targets.size() + optical_targets.size());
    all_targets.insert(all_targets.end(), radar_targets.begin(), radar_targets.end());
    all_targets.insert(all_targets.end(), thermal_targets.begin(), thermal_targets.end());
    all_targets.insert(all_targets.end(), optical_targets.begin(), optical_targets.end());
    
    // Sort by confidence to prioritize more confident targets
    std::sort(all_targets.begin(), all_targets.end(), 
              [](const Target& a, const Target& b) { return a.confidence > b.confidence; });
    
    // Pre-computed sensor weights for performance
    static const std::unordered_map<int, double> sensor_weights = {
        {static_cast<int>(TargetType::RADAR), 1.2},
        {static_cast<int>(TargetType::FUSED), 1.1},
        {static_cast<int>(TargetType::THERMAL), 0.9},
        {static_cast<int>(TargetType::OPTICAL), 1.0}
    };
    
    for (const auto& target : all_targets) {
        bool found_similar = false;
        
        // Early exit optimization - check only first few candidates
        size_t max_checks = std::min(fused_targets.size(), static_cast<size_t>(10));
        
        for (size_t i = 0; i < max_checks; ++i) {
            auto& fused = fused_targets[i];
            double distance = calculateDistance(target, fused);
            
            if (distance < fusion_threshold) {
                // Check for duplicates
                if (isDuplicateTarget(target, {fused})) {
                    found_similar = true;
                    break;
                }
                
                // Optimized weighted fusion
                double weight_fused = fused.confidence;
                double weight_new = target.confidence;
                
                double sensor_weight_fused = sensor_weights.at(static_cast<int>(fused.type));
                double sensor_weight_new = sensor_weights.at(static_cast<int>(target.type));
                
                double final_weight_fused = weight_fused * sensor_weight_fused;
                double final_weight_new = weight_new * sensor_weight_new;
                double final_total = final_weight_fused + final_weight_new;
                
                // SIMD-friendly weighted averages
                fused.x = (fused.x * final_weight_fused + target.x * final_weight_new) / final_total;
                fused.y = (fused.y * final_weight_fused + target.y * final_weight_new) / final_total;
                fused.z = (fused.z * final_weight_fused + target.z * final_weight_new) / final_total;
                fused.size = (fused.size * final_weight_fused + target.size * final_weight_new) / final_total;
                fused.velocity = (fused.velocity * final_weight_fused + target.velocity * final_weight_new) / final_total;
                
                // Improve confidence but with diminishing returns
                fused.confidence = std::min(fused.confidence + target.confidence * 0.2, 0.95);
                
                // Mark as fused target
                fused.type = TargetType::FUSED;
                found_similar = true;
                break;
            }
        }
        
        if (!found_similar) {
            Target new_target = target;
            new_target.threat_level = calculateThreatLevel(new_target);
            fused_targets.push_back(std::move(new_target));
        }
    }
    
    // Prioritize fused targets
    prioritizeTargets(fused_targets);
    
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
              << std::setw(10) << "Conf" << " | "
              << std::setw(8) << "Threat" << std::endl;
    std::cout << std::string(85, '-') << std::endl;
    
    for (const auto& target : detected_targets) {
std::cout << std::setw(4) << target.id << " | "
                  << std::setw(8) << typeToString(target.type) << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.x << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.y << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.z << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.velocity << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.size << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << target.confidence * 100 << "%" << " | "
                  << std::setw(8) << threatToString(target.threat_level)
                  << std::endl;
    }
    std::cout << std::endl;
}

ThreatLevel TargetDetector::calculateThreatLevel(const Target& target) {
    double distance = sqrt(target.x * target.x + target.y * target.y + target.z * target.z);
    double threat_score = 0.0;
    
    // Velocity contribution (40%)
    if (target.velocity > 50.0) threat_score += 0.4;
    else if (target.velocity > 20.0) threat_score += 0.3;
    else if (target.velocity > 10.0) threat_score += 0.2;
    else threat_score += 0.1;
    
    // Size contribution (20%)
    if (target.size > 5.0) threat_score += 0.2;
    else if (target.size > 2.0) threat_score += 0.15;
    else if (target.size > 1.0) threat_score += 0.1;
    else threat_score += 0.05;
    
    // Distance contribution (25%)
    if (distance < 10.0) threat_score += 0.25;
    else if (distance < 50.0) threat_score += 0.2;
    else if (distance < 100.0) threat_score += 0.15;
    else threat_score += 0.05;
    
    // Confidence contribution (15%)
    threat_score += target.confidence * 0.15;
    
    if (threat_score >= 0.8) return ThreatLevel::CRITICAL;
    if (threat_score >= 0.6) return ThreatLevel::HIGH;
    if (threat_score >= 0.4) return ThreatLevel::MEDIUM;
    return ThreatLevel::LOW;
}

void TargetDetector::updateTargetVelocity(Target& target, double dt) {
    if (dt <= 0) return;
    
    // Simple velocity tracking with some noise
    static double prev_x = target.x, prev_y = target.y, prev_z = target.z;
    static bool first_update = true;
    
    if (!first_update) {
        double dx = target.x - prev_x;
        double dy = target.y - prev_y;
        double dz = target.z - prev_z;
        
        double new_velocity = sqrt(dx*dx + dy*dy + dz*dz) / dt;
        
        // Smooth velocity update with exponential moving average
        const double alpha = 0.3;
        target.velocity = alpha * new_velocity + (1.0 - alpha) * target.velocity;
    }
    
    prev_x = target.x;
    prev_y = target.y;
    prev_z = target.z;
    first_update = false;
}

bool TargetDetector::isDuplicateTarget(const Target& new_target, const std::vector<Target>& existing_targets) {
    const double distance_threshold = 2.0;
    const double velocity_threshold = 5.0;
    const double size_threshold = 0.5;
    
    for (const auto& existing : existing_targets) {
        double distance = calculateDistance(new_target, existing);
        
        if (distance < distance_threshold) {
            double velocity_diff = std::abs(new_target.velocity - existing.velocity);
            double size_diff = std::abs(new_target.size - existing.size);
            
            if (velocity_diff < velocity_threshold && size_diff < size_threshold) {
                return true;
            }
        }
    }
    
    return false;
}

void TargetDetector::prioritizeTargets(std::vector<Target>& targets) {
    std::sort(targets.begin(), targets.end(), [](const Target& a, const Target& b) {
        // Sort by threat level first, then by confidence
        if (a.threat_level != b.threat_level) {
            return static_cast<int>(a.threat_level) > static_cast<int>(b.threat_level);
        }
        return a.confidence > b.confidence;
    });
    
    // Update threat levels after prioritization
    for (auto& target : targets) {
        target.threat_level = calculateThreatLevel(target);
    }
}

std::vector<Target> TargetDetector::getHighPriorityTargets() const {
    std::vector<Target> high_priority;
    
    for (const auto& target : detected_targets) {
        if (target.threat_level == ThreatLevel::HIGH || 
            target.threat_level == ThreatLevel::CRITICAL ||
            target.confidence > 0.8) {
            high_priority.push_back(target);
        }
    }
    
    return high_priority;
}

void TargetDetector::predictTargetPositions(std::vector<Target>& targets, double time_ahead) {
    for (auto& target : targets) {
        // Simple linear prediction
        double distance = sqrt(target.x * target.x + target.y * target.y + target.z * target.z);
        
        if (distance > 0) {
            // Normalize direction vector
            double dir_x = target.x / distance;
            double dir_y = target.y / distance;
            double dir_z = target.z / distance;
            
            // Predict new position
            target.x += dir_x * target.velocity * time_ahead;
            target.y += dir_y * target.velocity * time_ahead;
            target.z += dir_z * target.velocity * time_ahead;
            
            // Reduce confidence for predicted positions
            target.confidence *= 0.95;
        }
    }
}

void TargetDetector::addTarget(const Target& target) {
    Target new_target = target;
    new_target.threat_level = calculateThreatLevel(new_target);
    new_target.detection_time = std::chrono::system_clock::now();
    
    detected_targets.push_back(new_target);
}

size_t TargetDetector::getTargetCount() const {
    return detected_targets.size();
}

std::vector<Target> TargetDetector::getTargetsByThreatLevel(ThreatLevel level) const {
    std::vector<Target> filtered;
    
    for (const auto& target : detected_targets) {
        if (target.threat_level == level) {
            filtered.push_back(target);
        }
    }
    
    return filtered;
}

void TargetDetector::setFusionThreshold(double threshold) {
    if (threshold > 0 && threshold < 100.0) {
        fusion_threshold = threshold;
    }
}

void TargetDetector::setNoiseThreshold(double threshold) {
    if (threshold >= 0 && threshold <= 1.0) {
        noise_threshold = threshold;
    }
}

double TargetDetector::getFusionThreshold() const {
    return fusion_threshold;
}

double TargetDetector::getNoiseThreshold() const {
    return noise_threshold;
}