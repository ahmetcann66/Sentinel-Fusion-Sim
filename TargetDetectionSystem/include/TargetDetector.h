#ifndef TARGET_DETECTOR_H
#define TARGET_DETECTOR_H

#include <vector>
#include <cmath>
#include <string>

struct Target {
    int id;
    double x, y, z;
    double velocity;
    double size;
    double confidence;
    std::string type;
};

class TargetDetector {
public:
    std::vector<Target> detected_targets;
    
private:
    int next_target_id;
    
    double calculateDistance(const Target& t1, const Target& t2);
    bool isValidTarget(double confidence, double size);
    
public:
    TargetDetector();
    
    // Detection methods
    std::vector<Target> detectRadarTargets(const std::vector<std::vector<double>>& radar_data);
    std::vector<Target> detectThermalTargets(const std::vector<std::vector<double>>& thermal_data);
    std::vector<Target> detectOpticalTargets(const std::vector<std::vector<double>>& optical_data);
    
    // Processing methods
    void filterNoise(std::vector<Target>& targets);
    void trackTargets(std::vector<Target>& current_targets);
    std::vector<Target> fuseSensors(const std::vector<Target>& radar_targets,
                                   const std::vector<Target>& thermal_targets,
                                   const std::vector<Target>& optical_targets);
    
    // Utility methods
    void clearTargets();
    std::vector<Target> getDetectedTargets() const;
    void printTargets() const;
};

#endif 