#ifndef TARGET_DETECTOR_H
#define TARGET_DETECTOR_H

#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <memory>

enum class TargetType {
    RADAR,
    THERMAL,
    OPTICAL,
    FUSED
};

enum class ThreatLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct Target {
    int id;
    double x, y, z;
    double velocity;
    double size;
    double confidence;
    TargetType type;
    ThreatLevel threat_level;
    std::chrono::system_clock::time_point detection_time;
    std::string description;

class TargetDetector {
private:
    std::vector<Target> detected_targets;
    int next_target_id;
    double fusion_threshold;
    double noise_threshold;
    
    // Private helper methods
    double calculateDistance(const Target& t1, const Target& t2);
    bool isValidTarget(const Target& target);
    ThreatLevel calculateThreatLevel(const Target& target);
    std::string typeToString(TargetType type);
    std::string threatToString(ThreatLevel level);
    void updateTargetVelocity(Target& target, double dt);
    bool isDuplicateTarget(const Target& new_target, const std::vector<Target>& existing_targets);
    
public:
    explicit TargetDetector(double fusion_thresh = 5.0, double noise_thresh = 0.3);
    
    // Detection methods
    std::vector<Target> detectRadarTargets(const std::vector<std::vector<double>>& radar_data);
    std::vector<Target> detectThermalTargets(const std::vector<std::vector<double>>& thermal_data);
    std::vector<Target> detectOpticalTargets(const std::vector<std::vector<double>>& optical_data);
    
    // Processing methods
    void filterNoise(std::vector<Target>& targets);
    void trackTargets(std::vector<Target>& current_targets, double time_delta = 0.1);
    std::vector<Target> fuseSensors(const std::vector<Target>& radar_targets,
                                   const std::vector<Target>& thermal_targets,
                                   const std::vector<Target>& optical_targets);
    
    // Advanced processing
    void prioritizeTargets(std::vector<Target>& targets);
    std::vector<Target> getHighPriorityTargets() const;
    void predictTargetPositions(std::vector<Target>& targets, double time_ahead);
    
    // Utility methods
    void clearTargets();
    std::vector<Target> getDetectedTargets() const;
    void printTargets() const;
    size_t getTargetCount() const;
    std::vector<Target> getTargetsByThreatLevel(ThreatLevel level) const;
    
    // Configuration methods
    void setFusionThreshold(double threshold);
    void setNoiseThreshold(double threshold);
    double getFusionThreshold() const;
    double getNoiseThreshold() const;
};

#endif 