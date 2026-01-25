#include "MonitorInterface.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

MonitorInterface::MonitorInterface() : running_(false), detector_ptr_(nullptr) {}

MonitorInterface::~MonitorInterface() {
    stopMonitoring();
}

void MonitorInterface::startMonitoring(TargetDetector& detector) {
    detector_ptr_ = &detector;
    running_ = true;
    start_time_ = std::chrono::system_clock::now();

    monitor_thread_ = std::thread([this]() {
        std::vector<std::vector<double>> mock_data;
        
        while (running_) {
            clearScreen();
            displayHeader();
            
            // Mock data generate et
            mock_data.clear();
            std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<> pos_dist(0.0, 2000.0);
            std::uniform_real_distribution<> vel_dist(-300.0, 300.0);
            
            int signal_count = 5 + (gen() % 10);
            for(int i = 0; i < signal_count; ++i) {
                mock_data.push_back({
                    pos_dist(gen),
                    pos_dist(gen), 
                    pos_dist(gen),
                    vel_dist(gen)
                });
            }

            auto start = std::chrono::high_resolution_clock::now();
            auto targets = detector_ptr_->detectRadarTargets(mock_data);
            auto end = std::chrono::high_resolution_clock::now();
            
            double detection_time = std::chrono::duration<double, std::milli>(end - start).count();
            total_detections_++;
            avg_detection_time_ = (avg_detection_time_ * (total_detections_ - 1) + detection_time) / total_detections_;

            displayStats(targets, detection_time);
            displayTargets(targets);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            // ESC tuşu kontrolü
            #ifdef _WIN32
            if (_kbhit()) {
                if (_getch() == 27) break;
            }
            #else
            int ch = getch();
            if (ch == 27) break;
            #endif
        }
        running_ = false;
    });
}

void MonitorInterface::stopMonitoring() {
    running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

void MonitorInterface::clearScreen() const {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void MonitorInterface::displayHeader() const {
    std::cout << getColorCode(ThreatLevel::HIGH);
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    🎯 RADAR MONITORING SYSTEM                ║\n";
    std::cout << "║                   Sentinel Fusion Simulator                  ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    std::cout << resetColor();
    std::cout << "\n";
}

void MonitorInterface::displayStats(const std::vector<Target>& targets, double detection_time_ms) {
    auto stats = calculateThreatStats(targets);
    auto elapsed = std::chrono::duration<double>(std::chrono::system_clock::now() - start_time_).count();
    
    std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
    std::cout << "│ 📊 SYSTEM STATUS                                              │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ Runtime: " << std::setw(5) << std::fixed << std::setprecision(1) << elapsed << "s";
    std::cout << "  │ Detection Time: " << std::setw(5) << detection_time_ms << "ms │\n";
    std::cout << "│ Avg Time: " << std::setw(6) << std::fixed << std::setprecision(2) << avg_detection_time_ << "ms";
    std::cout << "  │ Total Scans: " << std::setw(7) << total_detections_ << "    │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    
    std::cout << "│ 🎯 THREAT DISTRIBUTION                                       │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    
    std::cout << "│ ";
    std::cout << getColorCode(ThreatLevel::CRITICAL) << "█CRITICAL: " << std::setw(2) << stats.critical << resetColor();
    std::cout << " │ ";
    std::cout << getColorCode(ThreatLevel::HIGH) << "   HIGH: " << std::setw(2) << stats.high << resetColor();
    std::cout << " │ ";
    std::cout << getColorCode(ThreatLevel::MEDIUM) << " MEDIUM: " << std::setw(2) << stats.medium << resetColor();
    std::cout << " │ ";
    std::cout << getColorCode(ThreatLevel::LOW) << "    LOW: " << std::setw(2) << stats.low << resetColor();
    std::cout << " │\n";
    
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ 🎮 CONTROLS: ESC to stop monitoring                           │\n";
    std::cout << "└─────────────────────────────────────────────────────────────┘\n\n";
}

void MonitorInterface::displayTargets(const std::vector<Target>& targets) {
    if (targets.empty()) {
        std::cout << getColorCode(ThreatLevel::LOW) << "⚪ No targets detected\n" << resetColor();
        return;
    }

    std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
    std::cout << "│ 🎯 DETECTED TARGETS (" << std::setw(2) << targets.size() << ")                                            │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    
    int max_display = std::min(10, (int)targets.size());
    for (int i = 0; i < max_display; ++i) {
        const auto& t = targets[i];
        std::cout << "│ ";
        std::cout << getColorCode(t.threat_level) << "█" << resetColor();
        std::cout << " ID:" << std::setw(3) << t.id;
        std::cout << " | Conf:" << std::setw(5) << std::fixed << std::setprecision(3) << t.confidence;
        std::cout << " | Vel:" << std::setw(6) << std::fixed << std::setprecision(1) << t.velocity << "m/s";
        std::cout << " | Pos:(" << std::setw(4) << std::fixed << std::setprecision(0) << t.x << ",";
        std::cout << std::setw(4) << std::fixed << std::setprecision(0) << t.y << ",";
        std::cout << std::setw(4) << std::fixed << std::setprecision(0) << t.z << ") │\n";
    }
    
    if (targets.size() > max_display) {
        std::cout << "│ ... and " << (targets.size() - max_display) << " more targets                      │\n";
    }
    
    std::cout << "└─────────────────────────────────────────────────────────────┘\n";
}

MonitorInterface::ThreatStats MonitorInterface::calculateThreatStats(const std::vector<Target>& targets) const {
    ThreatStats stats;
    for (const auto& t : targets) {
        switch (t.threat_level) {
            case ThreatLevel::CRITICAL: stats.critical++; break;
            case ThreatLevel::HIGH:     stats.high++; break;
            case ThreatLevel::MEDIUM:   stats.medium++; break;
            case ThreatLevel::LOW:      stats.low++; break;
        }
    }
    return stats;
}

std::string MonitorInterface::getColorCode(ThreatLevel level) const {
    #ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (level) {
        case ThreatLevel::CRITICAL: SetConsoleTextAttribute(hConsole, 12); break; // Red
        case ThreatLevel::HIGH:     SetConsoleTextAttribute(hConsole, 14); break; // Yellow  
        case ThreatLevel::MEDIUM:   SetConsoleTextAttribute(hConsole, 11); break; // Cyan
        case ThreatLevel::LOW:      SetConsoleTextAttribute(hConsole, 10); break; // Green
    }
    return "";
    #else
    switch (level) {
        case ThreatLevel::CRITICAL: return "\033[1;31m"; // Red
        case ThreatLevel::HIGH:     return "\033[1;33m"; // Yellow
        case ThreatLevel::MEDIUM:   return "\033[1;36m"; // Cyan
        case ThreatLevel::LOW:      return "\033[1;32m"; // Green
    }
    return "";
    #endif
}

std::string MonitorInterface::resetColor() const {
    #ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7); // White
    return "";
    #else
    return "\033[0m";
    #endif
}

#ifndef _WIN32
int MonitorInterface::getch() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    return ch;
}
#endif