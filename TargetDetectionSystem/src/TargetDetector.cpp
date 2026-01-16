#include "TargetDetector.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <map>
#include <random>
#include <execution>
#include <future>
#include <format>

// Modern string conversion methods with noexcept
std::string TargetDetector::typeToString(TargetType type) const noexcept {
    switch (type) {
        case TargetType::RADAR: return "Radar";
        case TargetType::THERMAL: return "Thermal";
        case TargetType::OPTICAL: return "Optical";
        case TargetType::FUSED: return "Fused";
        default: return "Unknown";
    }
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

std::string TargetDetector::statusToString(SensorStatus status) const noexcept {
    switch (status) {
        case SensorStatus::ACTIVE: return "Active";
        case SensorStatus::INACTIVE: return "Inactive";
        case SensorStatus::MAINTENANCE: return "Maintenance";
        case SensorStatus::ERROR: return "Error";
        default: return "Unknown";
    }
}

// Modern constructor with member initializer list
TargetDetector::TargetDetector(double fusion_thresh, double noise_thresh) 
    : next_target_id(1), fusion_threshold(fusion_thresh), noise_threshold(noise_thresh) {
    target_history.reserve(1000); // Pre-allocate for better performance
    detected_targets.reserve(500);
}

// Optimized distance calculation with noexcept
double TargetDetector::calculateDistance(const Target& t1, const Target& t2) const noexcept {
    const double dx = t1.x - t2.x;
    const double dy = t1.y - t2.y;
    const double dz = t1.z - t2.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

// Thread-safe target validation
bool TargetDetector::isValidTarget(const Target& target) const noexcept {
    return target.confidence > noise_threshold && target.size > 0.1;
}

// Modern radar detection with span and move semantics
std::vector<Target> TargetDetector::detectRadarTargets(std::span<const std::vector<double>> radar_data) {
    const auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<Target> targets;
    
    if (radar_data.empty() || radar_status != SensorStatus::ACTIVE) {
        return targets;
    }
    
    targets.reserve(radar_data.size() / 2); // Pre-allocate memory
    
    std::mutex target_mutex;
    
    // Parallel processing for better performance
    std::for_each(std::execution::par, radar_data.begin(), radar_data.end(),
        [&](const std::vector<double>& reading) {
            if (reading.size() >= 4) {
                const double x = reading[0];
                const double y = reading[1];
                const double z = reading[2];
                const double signal_strength = reading[3];
                
                if (signal_strength > noise_threshold) {
                    Target target{
                        0, // id will be set below
                        x, y, z,
                        std::sqrt(x*x + y*y) * 0.1,
                        signal_strength * 2.0,
                        std::min(signal_strength * 1.5, 1.0),
                        TargetType::RADAR,
                        ThreatLevel::LOW, // Will be calculated
                        "Radar detection"
                    };
                    
                    target.threat_level = calculateThreatLevel(target);
                    
                    if (isValidTarget(target)) {
                        std::lock_guard<std::mutex> lock(target_mutex);
                        target.id = next_target_id++;
                        targets.emplace_back(std::move(target));
                    }
                }
            }
        });
    
    // Update metrics
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    last_metrics = DetectionMetrics{
        static_cast<double>(duration.count()) / 1000.0,
        targets.size(),
        targets.empty() ? 0.0 : std::ranges::fold_left(
            targets | std::views::transform([](const Target& t) { return t.confidence; }), 
            0.0, std::plus<>()) / targets.size(),
        0, 0
    };
    
    return targets;
}

// Modern thermal detection with span
std::vector<Target> TargetDetector::detectThermalTargets(std::span<const std::vector<double>> thermal_data) {
    const auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<Target> targets;
    
    if (thermal_data.empty() || thermal_status != SensorStatus::ACTIVE) {
        return targets;
    }
    
    targets.reserve(thermal_data.size() / 2);
    
    std::mutex target_mutex;
    
    std::for_each(std::execution::par, thermal_data.begin(), thermal_data.end(),
        [&](const std::vector<double>& reading) {
            if (reading.size() >= 4) {
                const double x = reading[0];
                const double y = reading[1];
                const double z = reading[2];
                const double temperature = reading[3];
                
                if (temperature > 25.0) {
                    Target target{
                        0,
                        x, y, z,
                        std::sqrt(x*x + y*y) * 0.05,
                        std::max((temperature - 20.0) * 0.3, 0.5),
                        std::min((temperature - 20.0) / 20.0, 1.0),
                        TargetType::THERMAL,
                        ThreatLevel::LOW,
                        "Thermal detection"
                    };
                    
                    target.threat_level = calculateThreatLevel(target);
                    
                    if (isValidTarget(target)) {
                        std::lock_guard<std::mutex> lock(target_mutex);
                        target.id = next_target_id++;
                        targets.emplace_back(std::move(target));
                    }
                }
            }
        });
    
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    last_metrics = DetectionMetrics{
        static_cast<double>(duration.count()) / 1000.0,
        targets.size(),
        targets.empty() ? 0.0 : std::ranges::fold_left(
            targets | std::views::transform([](const Target& t) { return t.confidence; }), 
            0.0, std::plus<>()) / targets.size(),
        0, 0
    };
    
    return targets;
}

// Modern optical detection with span
std::vector<Target> TargetDetector::detectOpticalTargets(std::span<const std::vector<double>> optical_data) {
    const auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<Target> targets;
    
    if (optical_data.empty() || optical_status != SensorStatus::ACTIVE) {
        return targets;
    }
    
    targets.reserve(optical_data.size() / 2);
    
    std::mutex target_mutex;
    
    std::for_each(std::execution::par, optical_data.begin(), optical_data.end(),
        [&](const std::vector<double>& reading) {
            if (reading.size() >= 5) {
                const double x = reading[0];
                const double y = reading[1];
                const double z = reading[2];
                const double brightness = reading[3];
                const double contrast = reading[4];
                
                const double optical_confidence = brightness * contrast;
                
                if (optical_confidence > 0.2) {
                    Target target{
                        0,
                        x, y, z,
                        std::sqrt(x*x + y*y) * 0.08,
                        brightness * 3.0,
                        std::min(optical_confidence * 2.0, 1.0),
                        TargetType::OPTICAL,
                        ThreatLevel::LOW,
                        "Optical detection"
                    };
                    
                    target.threat_level = calculateThreatLevel(target);
                    
                    if (isValidTarget(target)) {
                        std::lock_guard<std::mutex> lock(target_mutex);
                        target.id = next_target_id++;
                        targets.emplace_back(std::move(target));
                    }
                }
            }
        });
    
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    last_metrics = DetectionMetrics{
        static_cast<double>(duration.count()) / 1000.0,
        targets.size(),
        targets.empty() ? 0.0 : std::ranges::fold_left(
            targets | std::views::transform([](const Target& t) { return t.confidence; }), 
            0.0, std::plus<>()) / targets.size(),
        0, 0
    };
    
    return targets;
}

// Thread-safe noise filtering
void TargetDetector::filterNoise(std::vector<Target>& targets) noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    
    targets.erase(
        std::remove_if(targets.begin(), targets.end(),
            [this](const Target& target) { return !isValidTarget(target); }),
        targets.end()
    );
}

// Enhanced target tracking with velocity prediction
void TargetDetector::trackTargets(std::vector<Target>& current_targets, double time_delta) noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    
    for (auto& target : current_targets) {
        // Check if target exists in history
        auto it = target_history.find(target.id);
        if (it != target_history.end()) {
            // Calculate new velocity based on position change
            const Target& old_target = it->second;
            const double dt = std::chrono::duration<double>(
                target.detection_time - old_target.detection_time).count();
            
            if (dt > 0) {
                const double vx = (target.x - old_target.x) / dt;
                const double vy = (target.y - old_target.y) / dt;
                const double vz = (target.z - old_target.z) / dt;
                target.velocity = std::sqrt(vx*vx + vy*vy + vz*vz);
            }
        }
        
        // Update target history
        target_history[target.id] = target;
        target_history[target.id].detection_time = std::chrono::system_clock::now();
    }
}

// Modern sensor fusion with spans
std::vector<Target> TargetDetector::fuseSensors(
    std::span<const Target> radar_targets,
    std::span<const Target> thermal_targets,
    std::span<const Target> optical_targets) {
    
    std::vector<Target> fused_targets;
    fused_targets.reserve(radar_targets.size() + thermal_targets.size() + optical_targets.size());
    
    // Add all radar targets
    for (const auto& target : radar_targets) {
        if (isValidTarget(target)) {
            fused_targets.push_back(target);
        }
    }
    
    // Fuse with thermal targets
    for (const auto& thermal : thermal_targets) {
        if (!isValidTarget(thermal)) continue;
        
        bool is_duplicate = false;
        for (auto& radar : fused_targets) {
            if (radar.type == TargetType::RADAR) {
                const double distance = calculateDistance(radar, thermal);
                if (distance < fusion_threshold) {
                    // Create fused target
                    radar.confidence = std::min(0.9, radar.confidence + thermal.confidence * 0.3);
                    radar.threat_level = std::max(radar.threat_level, thermal.threat_level);
                    radar.type = TargetType::FUSED;
                    radar.description += " + Thermal";
                    is_duplicate = true;
                    break;
                }
            }
        }
        
        if (!is_duplicate) {
            fused_targets.push_back(thermal);
        }
    }
    
    // Fuse with optical targets
    for (const auto& optical : optical_targets) {
        if (!isValidTarget(optical)) continue;
        
        bool is_duplicate = false;
        for (auto& target : fused_targets) {
            const double distance = calculateDistance(target, optical);
            if (distance < fusion_threshold) {
                target.confidence = std::min(0.95, target.confidence + optical.confidence * 0.2);
                target.threat_level = std::max(target.threat_level, optical.threat_level);
                if (target.type != TargetType::FUSED) {
                    target.type = TargetType::FUSED;
                    target.description += " + Optical";
                }
                is_duplicate = true;
                break;
            }
        }
        
        if (!is_duplicate) {
            fused_targets.push_back(optical);
        }
    }
    
    return fused_targets;
}

// Thread-safe target addition
void TargetDetector::addTarget(Target&& target) {
    std::lock_guard<std::mutex> lock(detection_mutex);
    detected_targets.emplace_back(std::move(target));
    total_detections++;
}

void TargetDetector::addTarget(const Target& target) {
    std::lock_guard<std::mutex> lock(detection_mutex);
    detected_targets.push_back(target);
    total_detections++;
}

// Thread-safe target clearing
void TargetDetector::clearTargets() noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    detected_targets.clear();
    target_history.clear();
    next_target_id = 1;
}

// Thread-safe target retrieval
std::vector<Target> TargetDetector::getDetectedTargets() const {
    std::lock_guard<std::mutex> lock(detection_mutex);
    return detected_targets;
}

// Thread-safe target count
size_t TargetDetector::getTargetCount() const noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    return detected_targets.size();
}

// Modern threat level calculation
ThreatLevel TargetDetector::calculateThreatLevel(const Target& target) const noexcept {
    if (target.velocity > 100.0 || target.confidence > 0.9) {
        return ThreatLevel::CRITICAL;
    } else if (target.velocity > 50.0 || target.confidence > 0.7) {
        return ThreatLevel::HIGH;
    } else if (target.velocity > 20.0 || target.confidence > 0.5) {
        return ThreatLevel::MEDIUM;
    }
    return ThreatLevel::LOW;
}

// Thread-safe sensor status management
void TargetDetector::setSensorStatus(TargetType type, SensorStatus status) noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    switch (type) {
        case TargetType::RADAR: radar_status = status; break;
        case TargetType::THERMAL: thermal_status = status; break;
        case TargetType::OPTICAL: optical_status = status; break;
        default: break;
    }
}

SensorStatus TargetDetector::getSensorStatus(TargetType type) const noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    switch (type) {
        case TargetType::RADAR: return radar_status;
        case TargetType::THERMAL: return thermal_status;
        case TargetType::OPTICAL: return optical_status;
        default: return SensorStatus::ERROR;
    }
}

std::optional<DetectionMetrics> TargetDetector::getLastDetectionMetrics() const noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    return last_metrics;
}

double TargetDetector::getAverageProcessingTime() const noexcept {
    std::lock_guard<std::mutex> lock(detection_mutex);
    return last_metrics.processing_time_ms;
}

size_t TargetDetector::getTotalDetections() const noexcept {
    return total_detections.load();
}

// Configuration methods with noexcept
void TargetDetector::setFusionThreshold(double threshold) noexcept {
    fusion_threshold = threshold;
}

void TargetDetector::setNoiseThreshold(double threshold) noexcept {
    noise_threshold = threshold;
}

double TargetDetector::getFusionThreshold() const noexcept {
    return fusion_threshold;
}

double TargetDetector::getNoiseThreshold() const noexcept {
    return noise_threshold;
}

// Additional implementation methods...
std::vector<Target> TargetDetector::detectRadarTargetsOptimized(std::span<const std::vector<double>> radar_data) {
    return detectRadarTargets(radar_data);
}

std::vector<Target> TargetDetector::detectThermalTargetsOptimized(std::span<const std::vector<double>> thermal_data) {
    return detectThermalTargets(thermal_data);
}

std::vector<Target> TargetDetector::detectOpticalTargetsOptimized(std::span<const std::vector<double>> optical_data) {
    return detectOpticalTargets(optical_data);
}

void TargetDetector::prioritizeTargets(std::vector<Target>& targets) noexcept {
    std::sort(targets.begin(), targets.end(),
        [](const Target& a, const Target& b) {
            if (a.threat_level != b.threat_level) {
                return static_cast<uint8_t>(a.threat_level) > static_cast<uint8_t>(b.threat_level);
            }
            return a.confidence > b.confidence;
        });
}

std::vector<Target> TargetDetector::getHighPriorityTargets() const {
    std::vector<Target> high_priority;
    
    std::lock_guard<std::mutex> lock(detection_mutex);
    std::copy_if(detected_targets.begin(), detected_targets.end(),
        std::back_inserter(high_priority),
        [](const Target& t) { 
            return t.threat_level == ThreatLevel::HIGH || t.threat_level == ThreatLevel::CRITICAL; 
        });
    
    return high_priority;
}

void TargetDetector::predictTargetPositions(std::vector<Target>& targets, double time_ahead) noexcept {
    for (auto& target : targets) {
        target.x += target.velocity * time_ahead * 0.1;
        target.y += target.velocity * time_ahead * 0.05;
    }
}

std::vector<Target> TargetDetector::getTargetsByThreatLevel(ThreatLevel level) const {
    std::vector<Target> filtered;
    
    std::lock_guard<std::mutex> lock(detection_mutex);
    std::copy_if(detected_targets.begin(), detected_targets.end(),
        std::back_inserter(filtered),
        [level](const Target& t) { return t.threat_level == level; });
    
    return filtered;
}

void TargetDetector::printTargets() const {
    std::lock_guard<std::mutex> lock(detection_mutex);
    
    std::cout << "=== Detected Targets ===\n";
    std::cout << std::format("{:<5} {:<10} {:<10} {:<10} {:<10} {:<10} {:<10}\n",
        "ID", "Type", "X", "Y", "Z", "Velocity", "Threat");
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& target : detected_targets) {
        std::cout << std::format("{:<5} {:<10} {:<10.2f} {:<10.2f} {:<10.2f} {:<10.2f} {:<10}\n",
            target.id, typeToString(target.type), target.x, target.y, target.z,
            target.velocity, threatToString(target.threat_level));
    }
    
    std::cout << "Total targets: " << detected_targets.size() << "\n";
}

bool TargetDetector::isDuplicateTarget(const Target& new_target, const std::vector<Target>& existing_targets) const noexcept {
    return std::any_of(existing_targets.begin(), existing_targets.end(),
        [&](const Target& existing) {
            return calculateDistance(new_target, existing) < fusion_threshold;
        });
}

void TargetDetector::updateTargetVelocity(Target& target, double dt) noexcept {
    if (dt <= 0) return;
    
    auto it = target_history.find(target.id);
    if (it != target_history.end()) {
        const Target& old_target = it->second;
        const double vx = (target.x - old_target.x) / dt;
        const double vy = (target.y - old_target.y) / dt;
        const double vz = (target.z - old_target.z) / dt;
        target.velocity = std::sqrt(vx*vx + vy*vy + vz*vz);
    }
}