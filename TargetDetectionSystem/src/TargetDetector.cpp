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

// Optimized thread-safe target validation with likely/unlikely hints
[[nodiscard]] bool TargetDetector::isValidTarget(const Target& target) const noexcept {
    return [[likely]] target.confidence > noise_threshold && [[likely]] target.size > 0.1;
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
    
    // Optimized parallel processing with chunked execution
    const size_t chunk_size = std::max<size_t>(1, radar_data.size() / std::thread::hardware_concurrency());
    
    std::for_each(std::execution::par_unseq, radar_data.begin(), radar_data.end(),
        [&](const std::vector<double>& reading) {
            if ([[likely]] reading.size() >= 4) {
                const double x = reading[0];
                const double y = reading[1];
                const double z = reading[2];
                const double signal_strength = reading[3];
                
                if ([[likely]] signal_strength > noise_threshold) {
                    // Pre-calculate common values
                    const double distance = std::sqrt(x*x + y*y);
                    const double confidence = std::min(signal_strength * 1.5, 1.0);
                    
                    Target target{
                        0, // id will be set below
                        x, y, z,
                        distance * 0.1,
                        signal_strength * 2.0,
                        confidence,
                        TargetType::RADAR,
                        ThreatLevel::LOW, // Will be calculated
                        "Radar detection"
                    };
                    
                    target.threat_level = calculateThreatLevel(target);
                    
                    if (isValidTarget(target)) {
                        std::lock_guard<std::mutex> lock(target_mutex);
                        target.id = next_target_id.fetch_add(1, std::memory_order_relaxed);
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
    
    // Optimized parallel processing for thermal data
    std::for_each(std::execution::par_unseq, thermal_data.begin(), thermal_data.end(),
        [&](const std::vector<double>& reading) {
            if ([[likely]] reading.size() >= 4) {
                const double x = reading[0];
                const double y = reading[1];
                const double z = reading[2];
                const double temperature = reading[3];
                
                if ([[likely]] temperature > 25.0) {
                    // Pre-calculate for better performance
                    const double distance = std::sqrt(x*x + y*y);
                    const double temp_diff = temperature - 20.0;
                    
                    Target target{
                        0,
                        x, y, z,
                        distance * 0.05,
                        std::max(temp_diff * 0.3, 0.5),
                        std::min(temp_diff / 20.0, 1.0),
                        TargetType::THERMAL,
                        ThreatLevel::LOW,
                        "Thermal detection"
                    };
                    
                    target.threat_level = calculateThreatLevel(target);
                    
                    if (isValidTarget(target)) {
                        std::lock_guard<std::mutex> lock(target_mutex);
                        target.id = next_target_id.fetch_add(1, std::memory_order_relaxed);
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
    
    // Optimized parallel processing for optical data
    std::for_each(std::execution::par_unseq, optical_data.begin(), optical_data.end(),
        [&](const std::vector<double>& reading) {
            if ([[likely]] reading.size() >= 5) {
                const double x = reading[0];
                const double y = reading[1];
                const double z = reading[2];
                const double brightness = reading[3];
                const double contrast = reading[4];
                
                const double optical_confidence = brightness * contrast;
                
                if ([[likely]] optical_confidence > 0.2) {
                    // Pre-calculate distance for better performance
                    const double distance = std::sqrt(x*x + y*y);
                    
                    Target target{
                        0,
                        x, y, z,
                        distance * 0.08,
                        brightness * 3.0,
                        std::min(optical_confidence * 2.0, 1.0),
                        TargetType::OPTICAL,
                        ThreatLevel::LOW,
                        "Optical detection"
                    };
                    
                    target.threat_level = calculateThreatLevel(target);
                    
                    if (isValidTarget(target)) {
                        std::lock_guard<std::mutex> lock(target_mutex);
                        target.id = next_target_id.fetch_add(1, std::memory_order_relaxed);
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

// Enhanced target tracking with velocity prediction and batch processing
void TargetDetector::trackTargets(std::vector<Target>& current_targets, double time_delta) noexcept {
    if (current_targets.empty()) return;
    
    std::lock_guard<std::mutex> lock(detection_mutex);
    
    const auto now = std::chrono::system_clock::now();
    
    // Batch process targets for better cache locality
    for (auto& target : current_targets) {
        // Check if target exists in history
        auto it = target_history.find(target.id);
        if (it != target_history.end()) {
            // Calculate new velocity based on position change
            const Target& old_target = it->second;
            const double dt = std::chrono::duration<double>(
                target.detection_time - old_target.detection_time).count();
            
            if (dt > 0) [[likely]] {
                const double dx = target.x - old_target.x;
                const double dy = target.y - old_target.y;
                const double dz = target.z - old_target.z;
                // Use faster sqrt approximation for high-frequency calls
                target.velocity = std::sqrt(dx*dx + dy*dy + dz*dz) / dt;
            }
        }
        
        // Update target history with pre-calculated time
        target.detection_time = now;
        target_history[target.id] = target;
    }
}

// Optimized sensor fusion with spatial hashing for O(1) lookup
std::vector<Target> TargetDetector::fuseSensors(
    std::span<const Target> radar_targets,
    std::span<const Target> thermal_targets,
    std::span<const Target> optical_targets) {
    
    // Pre-allocate with better estimation
    std::vector<Target> fused_targets;
    fused_targets.reserve(std::max({radar_targets.size(), thermal_targets.size(), optical_targets.size()}));
    
    // Spatial hash grid for O(1) proximity queries
    const double grid_size = fusion_threshold;
    std::unordered_map<uint64_t, std::vector<size_t>> spatial_hash;
    
    // Lambda to convert position to grid hash
    auto get_grid_hash = [grid_size](double x, double y, double z) -> uint64_t {
        const int gx = static_cast<int>(std::floor(x / grid_size));
        const int gy = static_cast<int>(std::floor(y / grid_size));
        const int gz = static_cast<int>(std::floor(z / grid_size));
        return (static_cast<uint64_t>(gx) << 42) | (static_cast<uint64_t>(gy) << 21) | static_cast<uint64_t>(gz);
    };
    
    // Add all valid radar targets to spatial hash
    for (const auto& target : radar_targets) {
        if (isValidTarget(target)) {
            const size_t idx = fused_targets.size();
            fused_targets.push_back(target);
            spatial_hash[get_grid_hash(target.x, target.y, target.z)].push_back(idx);
        }
    }
    
    // Optimized thermal fusion with spatial hashing
    for (const auto& thermal : thermal_targets) {
        if (!isValidTarget(thermal)) continue;
        
        bool is_duplicate = false;
        const auto hash = get_grid_hash(thermal.x, thermal.y, thermal.z);
        
        // Check nearby grid cells for potential matches
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    const auto nearby_hash = hash + (dx << 42) + (dy << 21) + dz;
                    auto it = spatial_hash.find(nearby_hash);
                    if (it != spatial_hash.end()) {
                        for (size_t idx : it->second) {
                            auto& radar = fused_targets[idx];
                            if (radar.type == TargetType::RADAR) {
                                const double distance = calculateDistance(radar, thermal);
                                if (distance < fusion_threshold) [[unlikely]] {
                                    // Create fused target with optimized confidence calculation
                                    radar.confidence = std::min(0.9, radar.confidence + thermal.confidence * 0.3);
                                    radar.threat_level = std::max(radar.threat_level, thermal.threat_level);
                                    radar.type = TargetType::FUSED;
                                    radar.description += " + Thermal";
                                    is_duplicate = true;
                                    goto thermal_done;
                                }
                            }
                        }
                    }
                }
            }
        }
        thermal_done:
        if (!is_duplicate) {
            const size_t idx = fused_targets.size();
            fused_targets.push_back(thermal);
            spatial_hash[hash].push_back(idx);
        }
    }
    
    // Optimized optical fusion with spatial hashing
    for (const auto& optical : optical_targets) {
        if (!isValidTarget(optical)) continue;
        
        bool is_duplicate = false;
        const auto hash = get_grid_hash(optical.x, optical.y, optical.z);
        
        // Check nearby grid cells
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    const auto nearby_hash = hash + (dx << 42) + (dy << 21) + dz;
                    auto it = spatial_hash.find(nearby_hash);
                    if (it != spatial_hash.end()) {
                        for (size_t idx : it->second) {
                            auto& target = fused_targets[idx];
                            const double distance = calculateDistance(target, optical);
                            if (distance < fusion_threshold) [[unlikely]] {
                                target.confidence = std::min(0.95, target.confidence + optical.confidence * 0.2);
                                target.threat_level = std::max(target.threat_level, optical.threat_level);
                                if (target.type != TargetType::FUSED) {
                                    target.type = TargetType::FUSED;
                                    target.description += " + Optical";
                                }
                                is_duplicate = true;
                                goto optical_done;
                            }
                        }
                    }
                }
            }
        }
        optical_done:
        if (!is_duplicate) {
            const size_t idx = fused_targets.size();
            fused_targets.push_back(optical);
            spatial_hash[hash].push_back(idx);
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