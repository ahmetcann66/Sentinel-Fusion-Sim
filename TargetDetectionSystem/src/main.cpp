#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "../include/TargetDetector.h"

using namespace std::chrono_literals;

std::vector<std::vector<double>> generateMockData() {
    std::vector<std::vector<double>> data;
    
    static std::random_device rd;
    static std::mt19937 gen(rd()); 

    std::uniform_int_distribution<> count_dist(5, 20);
    int count = count_dist(gen);

    std::uniform_real_distribution<> pos_dist(-2500.0, 2500.0);
    std::uniform_real_distribution<> vel_dist(-400.0, 400.0);

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

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

int main() {
    TargetDetector detector(0.4);
    int scan_cycle = 1;

    std::cout << "--- SENTINEL RADAR SYSTEM ACTIVATED ---\n";
    std::this_thread::sleep_for(1s);

    while (true) {
        auto raw_data = generateMockData();
        auto targets = detector.detectRadarTargets(raw_data);

        clearScreen();
        std::cout << "========================================\n";
        std::cout << "      SENTINEL RADAR - LIVE FEED        \n";
        std::cout << "========================================\n";
        std::cout << "Scan Cycle: " << scan_cycle++ << "\n";
        std::cout << "Raw Signals Received: " << raw_data.size() << "\n";
        std::cout << "Active Targets Locked: " << detector.getTargetCount() << "\n";
        std::cout << "----------------------------------------\n";
        
        detector.printTargets();

        std::cout << "\n[Scanning for new threats...]\n";
        std::cout << "(Press Ctrl+C to abort simulation)\n";

        std::this_thread::sleep_for(1500ms);
    }

    return 0;
}