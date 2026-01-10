#include "../include/TargetDetector.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

class SensorSimulator {
public:
    static std::vector<std::vector<double>> generateRadarData(int num_targets) {
        std::vector<std::vector<double>> data;
        srand(time(0));
        
        for (int i = 0; i < num_targets; i++) {
            std::vector<double> reading;
            reading.push_back((rand() % 1000 - 500) / 10.0);
            reading.push_back((rand() % 1000 - 500) / 10.0);
            reading.push_back((rand() % 200) / 10.0);
            reading.push_back((rand() % 100) / 100.0);
            data.push_back(reading);
        }
        
        return data;
    }
    
    static std::vector<std::vector<double>> generateThermalData(int num_targets) {
        std::vector<std::vector<double>> data;
        srand(time(0));
        
        for (int i = 0; i < num_targets; i++) {
            std::vector<double> reading;
            reading.push_back((rand() % 800 - 400) / 10.0);
            reading.push_back((rand() % 800 - 400) / 10.0);
            reading.push_back((rand() % 150) / 10.0);
            reading.push_back(20.0 + (rand() % 400) / 10.0);
            data.push_back(reading);
        }
        
        return data;
    }
    
    static std::vector<std::vector<double>> generateOpticalData(int num_targets) {
        std::vector<std::vector<double>> data;
        srand(time(0));
        
        for (int i = 0; i < num_targets; i++) {
            std::vector<double> reading;
            reading.push_back((rand() % 600 - 300) / 10.0);
            reading.push_back((rand() % 600 - 300) / 10.0);
            reading.push_back((rand() % 100) / 10.0);
            reading.push_back((rand() % 100) / 100.0);
            reading.push_back((rand() % 100) / 100.0);
            data.push_back(reading);
        }
        
        return data;
    }
};

void demonstrateSystem() {
    TargetDetector detector;
    
    std::cout << "=== SAVUNMA SANAYİ HEDEF TESPİT SİSTEMİ ===" << std::endl;
    std::cout << "Sistem başlatılıyor..." << std::endl << std::endl;
    
    auto radar_data = SensorSimulator::generateRadarData(15);
    auto thermal_data = SensorSimulator::generateThermalData(12);
    auto optical_data = SensorSimulator::generateOpticalData(10);
    
    std::cout << "Sensor verileri toplandı." << std::endl;
    std::cout << "Radar okumaları: " << radar_data.size() << std::endl;
    std::cout << "Termal okumalar: " << thermal_data.size() << std::endl;
    std::cout << "Optik okumalar: " << optical_data.size() << std::endl << std::endl;
    
    auto radar_targets = detector.detectRadarTargets(radar_data);
    auto thermal_targets = detector.detectThermalTargets(thermal_data);
    auto optical_targets = detector.detectOpticalTargets(optical_data);
    
    std::cout << "Tespit edilen hedefler (onay öncesi):" << std::endl;
    std::cout << "Radar hedefleri: " << radar_targets.size() << std::endl;
    std::cout << "Termal hedefler: " << thermal_targets.size() << std::endl;
    std::cout << "Optik hedefler: " << optical_targets.size() << std::endl << std::endl;
    
    detector.filterNoise(radar_targets);
    detector.filterNoise(thermal_targets);
    detector.filterNoise(optical_targets);
    
    auto fused_targets = detector.fuseSensors(radar_targets, thermal_targets, optical_targets);
    detector.trackTargets(fused_targets);
    
    detector = TargetDetector();
    
    for (const auto& target : fused_targets) {
        detector.detected_targets.push_back(target);
    }
    
    detector.printTargets();
    
    std::cout << "=== SİSTEM ÖZETİ ===" << std::endl;
    std::cout << "Toplam füze hedefi: " << fused_targets.size() << std::endl;
    std::cout << "Yüksek öncelikli hedefler (>70% güven): " << std::count_if(fused_targets.begin(), fused_targets.end(),
                                                    [](const Target& t) { return t.confidence > 0.7; }) << std::endl;
    std::cout << "Hızlı hedefler (>5 m/s): " << std::count_if(fused_targets.begin(), fused_targets.end(),
                                           [](const Target& t) { return t.velocity > 5.0; }) << std::endl;
}

int main() {
    try {
        demonstrateSystem();
    } catch (const std::exception& e) {
        std::cerr << "Hata: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}