#ifndef TARGET_DETECTOR_H
#define TARGET_DETECTOR_H

#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <memory>
#include <iostream>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <random>

enum class ThreatLevel : uint8_t {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// C++20 concept for valid target data
template<typename T>
concept TargetData = requires(T t) {
    { t.x } -> std::convertible_to<double>;
    { t.y } -> std::convertible_to<double>;
    { t.z } -> std::convertible_to<double>;
    { t.confidence } -> std::convertible_to<double>;
};

// C++20 concept for radar data point
template<typename T>
concept RadarDataPoint = requires(T t) {
    { std::size(t) } -> std::convertible_to<std::size_t>;
    { t[0] } -> std::convertible_to<double>;
    { t[1] } -> std::convertible_to<double>;
    { t[2] } -> std::convertible_to<double>;
    { t[3] } -> std::convertible_to<double>;
};

struct Target {
    int id{};
    double x{}, y{}, z{};
    double velocity{};
    double confidence{};
    ThreatLevel threat_level{ThreatLevel::LOW};
    std::chrono::system_clock::time_point detection_time{std::chrono::system_clock::now()};
    std::string description{};
    
    Target() = default;
    
Target(int id, double x, double y, double z, double velocity, double confidence, 
           ThreatLevel threat_level, std::string description = "")
        : id(id), x(x), y(y), z(z), velocity(velocity), confidence(confidence),
          threat_level(threat_level), detection_time(std::chrono::system_clock::now()), 
          description(std::move(description)) {}
    
    // C++20 three-way comparison operator - optimized
    [[nodiscard]] std::strong_ordering operator<=>(const Target& other) const noexcept {
        if (threat_level != other.threat_level) {
            return threat_level <=> other.threat_level;
        }
        return confidence <=> other.confidence;
    }
    
    bool operator==(const Target& other) const noexcept = default;
};

// Simple sensor class
class SimpleSensor {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> noise_dist;
    double detection_range;
    double accuracy;
    
public:
    explicit SimpleSensor(double range = 100.0, double acc = 0.95) 
        : rng(std::random_device{}())
        , noise_dist(-0.1, 0.1)
        , detection_range(range), accuracy(acc) {}
    
    [[nodiscard]] bool canDetect(double distance) const noexcept {
        return distance <= detection_range;
    }
    
    [[nodiscard]] double getReading(double true_value) noexcept {
        if (static_cast<double>(rng()) / rng.max() > accuracy) {
            return true_value + noise_dist(rng);
        }
        return true_value;
    }
    
    [[nodiscard]] std::vector<double> scanForTargets(double center_x, double center_y) noexcept {
        std::vector<double> detections;
        detections.reserve(5);  // Pre-allocate known size
        
        for (int i = 0; i < 5; ++i) {
            double dx = center_x + i*10;
            double dy = center_y + i*10;
            double distance = std::hypot(dx, dy);  // More efficient than sqrt(pow)
            if (canDetect(distance)) {
                double detection = getReading(distance);
                if (detection > 0.1) {
                    detections.push_back(detection);
                }
            }
        }
        
        return detections;
    }
    
    void setRange(double new_range) noexcept { detection_range = new_range; }
    [[nodiscard]] double getRange() const noexcept { return detection_range; }
};

class TargetDetector {
private:
    std::vector<Target> detected_targets;
    int next_target_id{1};
    double noise_threshold{0.3};
    std::unique_ptr<SimpleSensor> sensor;
    
    [[nodiscard]] bool isValidTarget(const Target& target) const noexcept;
    [[nodiscard]] ThreatLevel calculateThreatLevel(const Target& target) const noexcept;
    std::string threatToString(ThreatLevel level) const noexcept;
    
public:
    explicit TargetDetector(double noise_thresh = 0.3);
    ~TargetDetector() = default;
    
    // Radar detection methods
    std::vector<Target> detectRadarTargets(const std::vector<std::vector<double>>& radar_data);
    
    // C++20 templated detection method with concepts
    template<std::ranges::range R>
    requires RadarDataPoint<std::ranges::range_value_t<R>>
    std::vector<Target> detectRadarTargets(const R& radar_data);
    
    // Processing methods
    void filterNoise(std::vector<Target>& targets) noexcept;
    void prioritizeTargets(std::vector<Target>& targets) noexcept;
    
    // Utility methods
    void clearTargets() noexcept;
    void addTarget(const Target& target);
    [[nodiscard]] std::vector<Target> getDetectedTargets() const;
    void printTargets() const;
    [[nodiscard]] size_t getTargetCount() const noexcept;
    
    // Configuration
    void setNoiseThreshold(double threshold) noexcept;
    [[nodiscard]] double getNoiseThreshold() const noexcept;
    
    // Sensor methods
    void setSensor(std::unique_ptr<SimpleSensor> new_sensor) noexcept;
    [[nodiscard]] SimpleSensor* getSensor() const noexcept;
    [[nodiscard]] std::vector<Target> detectWithSensor(double center_x, double center_y);
};

#endif