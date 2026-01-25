#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "TargetDetector.h"
#include "MonitorInterface.h"

void showMenu() {
    std::cout << "\n🎯 Sentinel Fusion Sim - Main Menu\n";
    std::cout << "1. Quick Test (Single Scan)\n";
    std::cout << "2. Real-time Monitoring\n";
    std::cout << "3. Benchmark Mode\n";
    std::cout << "4. Exit\n";
    std::cout << "Select option: ";
}

std::vector<std::vector<double>> generateMockData(int count) {
    std::vector<std::vector<double>> data;
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> pos_dist(0.0, 2000.0);
    std::uniform_real_distribution<> vel_dist(-300.0, 300.0);

    for(int i = 0; i < count; ++i) {
        data.push_back({
            pos_dist(gen),
            pos_dist(gen), 
            pos_dist(gen),
            vel_dist(gen)
        });
    }
    return data;
}

void quickTest() {
    std::cout << "\n🚀 Quick Test Mode\n";
    std::cout << "Enter threshold (0.1-0.9, default=0.3): ";
    
    double threshold = 0.3;
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        threshold = std::stod(input);
    }
    
    TargetDetector detector(threshold);
    auto raw_data = generateMockData(15);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto targets = detector.detectRadarTargets(raw_data);
    auto end = std::chrono::high_resolution_clock::now();
    
    double detection_time = std::chrono::duration<double, std::milli>(end - start).count();
    
    std::cout << "\n⏱️  Detection Time: " << detection_time << "ms\n";
    detector.printTargets();
}

void benchmarkMode() {
    std::cout << "\n📊 Benchmark Mode\n";
    std::cout << "Running performance tests...\n\n";
    
    TargetDetector detector(0.3);
    
    std::vector<int> test_sizes = {100, 500, 1000, 2000};
    
    for (int size : test_sizes) {
        auto data = generateMockData(size);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto targets = detector.detectRadarTargets(data);
        auto end = std::chrono::high_resolution_clock::now();
        
        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double throughput = size / (time_ms / 1000.0);
        
        std::cout << "Size: " << std::setw(4) << size 
                  << " | Time: " << std::setw(6) << std::fixed << std::setprecision(2) << time_ms << "ms"
                  << " | Targets: " << std::setw(3) << targets.size()
                  << " | Throughput: " << std::setw(8) << std::setprecision(0) << throughput << " targets/s\n";
    }
}

void realtimeMonitoring() {
    std::cout << "\n🎮 Real-time Monitoring\n";
    std::cout << "Starting monitoring mode... Press ESC to stop\n\n";
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    TargetDetector detector(0.25);
    MonitorInterface monitor;
    
    monitor.startMonitoring(detector);
    
    while (monitor.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n✅ Monitoring stopped successfully\n";
}

int main() {
    std::cout << "🎯 Sentinel Fusion Simulator v2.0\n";
    std::cout << "Advanced Radar Target Detection System\n\n";
    
    while (true) {
        showMenu();
        
        int choice;
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        try {
            choice = std::stoi(input);
            
            switch (choice) {
                case 1:
                    quickTest();
                    break;
                case 2:
                    realtimeMonitoring();
                    break;
                case 3:
                    benchmarkMode();
                    break;
                case 4:
                    std::cout << "👋 Exiting Sentinel Fusion Simulator...\n";
                    return 0;
                default:
                    std::cout << "❌ Invalid option. Please try again.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Invalid input. Please enter a number.\n";
        }
        
        if (choice != 4) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
    }
    
    return 0;
}