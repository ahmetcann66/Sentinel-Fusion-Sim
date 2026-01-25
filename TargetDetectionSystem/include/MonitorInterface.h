#ifndef MONITOR_INTERFACE_H
#define MONITOR_INTERFACE_H

#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <iomanip>
#include "TargetDetector.h"

class MonitorInterface {
public:
    MonitorInterface();
    ~MonitorInterface();

    void startMonitoring(TargetDetector& detector);
    void stopMonitoring();
    void updateDisplay(const std::vector<Target>& targets, double detection_time_ms);
    bool isRunning() const { return running_; }

private:
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    TargetDetector* detector_ptr_;
    std::chrono::system_clock::time_point start_time_;
    int total_detections_{0};
    double avg_detection_time_{0.0};

    void displayHeader() const;
    void displayStats(const std::vector<Target>& targets, double detection_time_ms);
    void displayTargets(const std::vector<Target>& targets);
    void displayProgressBar(double percentage) const;
    std::string getColorCode(ThreatLevel level) const;
    std::string resetColor() const;
    void clearScreen() const;
    std::string formatTime(std::chrono::system_clock::time_point time) const;
    
    #ifndef _WIN32
    int getch();
    #endif

    struct ThreatStats {
        int critical{0}, high{0}, medium{0}, low{0};
    };
    
    ThreatStats calculateThreatStats(const std::vector<Target>& targets) const;
};

#endif