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
    
    constexpr Target(int id, double x, double y, double z, double velocity, double confidence, 
                     ThreatLevel threat_level, std::string description = "")
        : id(id), x(x), y(y), z(z), velocity(velocity), confidence(confidence),
          threat_level(threat_level), detection_time(std::chrono::system_clock::now()), 
          description(std::move(description)) {}
    
    // C++20 three-way comparison operator
    auto operator<=>(const Target& other) const noexcept {
        if (auto cmp = threat_level <=> other.threat_level; cmp != 0) return cmp;
        return confidence <=> other.confidence;
    }
    
    bool operator==(const Target& other) const noexcept = default;
};

class TargetDetector {
private:
    std::vector<Target> detected_targets;
    int next_target_id{1};
    double noise_threshold{0.3};
    
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
};

#endif