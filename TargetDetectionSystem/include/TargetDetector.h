#ifndef TARGET_DETECTOR_H
#define TARGET_DETECTOR_H

#include <vector>
#include <string>
#include <chrono>
#include <compare>
#include <cmath>
#include <optional>
#include <ranges>
#include <algorithm>

enum class ThreatLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
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

    [[nodiscard]] std::partial_ordering operator<=>(const Target& other) const noexcept {
        if (auto cmp = threat_level <=> other.threat_level; cmp != 0) {
            return cmp;
        }
        return confidence <=> other.confidence;
    }

    bool operator==(const Target& other) const {
        return (*this <=> other) == 0;
    }
};

class TargetDetector {
public:
    explicit TargetDetector(double threshold = 0.5);

    std::vector<Target> detectRadarTargets(const std::vector<std::vector<double>>& raw_data);
    void printTargets() const;
    size_t getTargetCount() const;

private:
    double confidence_threshold_;
    std::vector<Target> detected_targets_;
    int next_target_id_{1};

    std::optional<Target> processSignal(const std::vector<double>& signal_point);
    ThreatLevel calculateThreat(double velocity, double distance) const;
};

#endif