#ifndef TARGET_DETECTOR_H
#define TARGET_DETECTOR_H

#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <memory>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <optional>
#include <variant>
#include <span>
#include <concepts>
#include <ranges>
#include <mutex>
#include <atomic>

enum class TargetType : uint8_t {
    RADAR = 0,
    THERMAL = 1,
    OPTICAL = 2,
    FUSED = 3
};

enum class ThreatLevel : uint8_t {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

enum class SensorStatus : uint8_t {
    ACTIVE = 0,
    INACTIVE = 1,
    MAINTENANCE = 2,
    ERROR = 3
};

// Memory-aligned Target structure for better cache performance
struct alignas(32) Target {
    int id{};
    double x{}, y{}, z{};
    double velocity{};
    double size{};
    double confidence{};
    TargetType type{TargetType::RADAR};
    ThreatLevel threat_level{ThreatLevel::LOW};
    std::chrono::system_clock::time_point detection_time{std::chrono::system_clock::now()};
    std::string description{};
    
    // Move semantics support
    Target() = default;
    Target(Target&&) = default;
    Target& operator=(Target&&) = default;
    Target(const Target&) = default;
    Target& operator=(const Target&) = default;
    
    // Constructor with parameters
    Target(int id, double x, double y, double z, double velocity, double size, 
           double confidence, TargetType type, ThreatLevel threat_level, 
           std::string description = "")
        : id(id), x(x), y(y), z(z), velocity(velocity), size(size),
          confidence(confidence), type(type), threat_level(threat_level),
          detection_time(std::chrono::system_clock::now()), 
          description(std::move(description)) {}
          
    // Memory pool allocation support
    void* operator new(std::size_t size);
    void operator delete(void* ptr) noexcept;
};

struct DetectionMetrics {
    double processing_time_ms{};
    size_t targets_detected{};
    double average_confidence{};
    size_t false_positives{};
    size_t missed_targets{};
};

class TargetDetector {
private:
    // Cache-friendly data structures with alignment and memory pool
    alignas(64) std::vector<Target> detected_targets;
    std::unordered_map<int, Target> target_history;
    std::atomic<int> next_target_id{1};
    alignas(64) double fusion_threshold{5.0};
    alignas(64) double noise_threshold{0.3};
    mutable std::mutex detection_mutex;
    SensorStatus radar_status{SensorStatus::ACTIVE};
    SensorStatus thermal_status{SensorStatus::ACTIVE};
    SensorStatus optical_status{SensorStatus::ACTIVE};
    
    // Memory pool for Target objects
    static constexpr size_t TARGET_POOL_SIZE = 1024;
    alignas(64) static thread_local std::array<Target, TARGET_POOL_SIZE> target_pool;
    static thread_local size_t pool_index;
    
    // Private helper methods with optimization hints
    [[nodiscard]] double calculateDistance(const Target& t1, const Target& t2) const noexcept;
    [[nodiscard]] bool isValidTarget(const Target& target) const noexcept;
    [[nodiscard]] ThreatLevel calculateThreatLevel(const Target& target) const noexcept;
    std::string typeToString(TargetType type) const noexcept;
    std::string threatToString(ThreatLevel level) const noexcept;
    std::string statusToString(SensorStatus status) const noexcept;
    void updateTargetVelocity(Target& target, double dt) noexcept;
    bool isDuplicateTarget(const Target& new_target, const std::vector<Target>& existing_targets) const noexcept;
    
    // Modern C++ helper methods
    template<typename Container>
    requires std::ranges::range<Container>
    auto findTargetsByType(Container&& targets, TargetType type) const;
    
public:
    explicit TargetDetector(double fusion_thresh = 5.0, double noise_thresh = 0.3);
    ~TargetDetector() = default;
    
    // Rule of Five
    TargetDetector(const TargetDetector&) = delete;
    TargetDetector& operator=(const TargetDetector&) = delete;
    TargetDetector(TargetDetector&&) = default;
    TargetDetector& operator=(TargetDetector&&) = default;
    
    // Detection methods with modern C++ signatures
    std::vector<Target> detectRadarTargets(std::span<const std::vector<double>> radar_data);
    std::vector<Target> detectThermalTargets(std::span<const std::vector<double>> thermal_data);
    std::vector<Target> detectOpticalTargets(std::span<const std::vector<double>> optical_data);
    
    // Optimized detection methods with move semantics and parallel processing
    std::vector<Target> detectRadarTargetsOptimized(std::span<const std::vector<double>> radar_data);
    std::vector<Target> detectThermalTargetsOptimized(std::span<const std::vector<double>> thermal_data);
    std::vector<Target> detectOpticalTargetsOptimized(std::span<const std::vector<double>> optical_data);
    
    // Processing methods
    void filterNoise(std::vector<Target>& targets) noexcept;
    void trackTargets(std::vector<Target>& current_targets, double time_delta = 0.1) noexcept;
    std::vector<Target> fuseSensors(std::span<const Target> radar_targets,
                                   std::span<const Target> thermal_targets,
                                   std::span<const Target> optical_targets);
    
    // Advanced processing
    void prioritizeTargets(std::vector<Target>& targets) noexcept;
    std::vector<Target> getHighPriorityTargets() const;
    void predictTargetPositions(std::vector<Target>& targets, double time_ahead) noexcept;
    
    // Thread-safe utility methods
    void clearTargets() noexcept;
    void addTarget(Target&& target);
    void addTarget(const Target& target);
    [[nodiscard]] std::vector<Target> getDetectedTargets() const;
    void printTargets() const;
    [[nodiscard]] size_t getTargetCount() const noexcept;
    [[nodiscard]] std::vector<Target> getTargetsByThreatLevel(ThreatLevel level) const;
    
    // Configuration methods
    void setFusionThreshold(double threshold) noexcept;
    void setNoiseThreshold(double threshold) noexcept;
    [[nodiscard]] double getFusionThreshold() const noexcept;
    [[nodiscard]] double getNoiseThreshold() const noexcept;
    
    // New sensor status methods
    void setSensorStatus(TargetType type, SensorStatus status) noexcept;
    [[nodiscard]] SensorStatus getSensorStatus(TargetType type) const noexcept;
    [[nodiscard]] std::optional<DetectionMetrics> getLastDetectionMetrics() const noexcept;
    
    // Performance monitoring
    [[nodiscard]] double getAverageProcessingTime() const noexcept;
    [[nodiscard]] size_t getTotalDetections() const noexcept;
private:
    mutable DetectionMetrics last_metrics;
    mutable size_t total_detections{};
    
    template<typename Container>
    requires std::ranges::range<Container>
    auto findTargetsByType(Container&& targets, TargetType type) const {
        std::vector<Target> result;
        for (const auto& target : targets) {
            if (target.type == type) {
                result.push_back(target);
            }
        }
        return result;
    }
};

#endif 